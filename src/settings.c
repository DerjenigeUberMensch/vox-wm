#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "FNotify/fnotify.h"

#include "settings.h"
#include "util.h"
#include "safebool.h"
#include "config.h"
#include "file_util.h"
#include "threading.h"
#include "main.h"




static int 
US_LOCK_MUTEX(UserSettings *us)
{
    if(!us)
    {   return -1;
    }

    if(us->use_threads)
    {   return pthread_mutex_lock(&us->mutex);
    }

    return -1;
}

static int
US_UNLOCK_MUTEX(UserSettings *us)
{
    if(!us)
    {   return -1;
    }

    if(us->use_threads)
    {   return pthread_mutex_unlock(&us->mutex);
    }
    
    return -1;
}


void
USSetupCFGVars(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    SCParser *cfg = us->cfg;
    const SCSetting *usdata = us->holder;

    i32 i;
    u8 err = 0;
    const int READONLY = 1;
    /* global settings */
    for(i = 0; i < UserSettingsLAST; ++i)
    {   
        err = SCParserNewVar(cfg, usdata[i].name, usdata[i].name_len, READONLY, 0, usdata[i].type);
        if(err)
        {   DebugWarn("Failed to create: \"%s\"", usdata[i].name);
        }
    }
}

void
USSetupCFGDefaults(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    Generic *data;
    i32 i;

    SCSetting *usdata = us->holder;

    for(i = 0; i < UserSettingsLAST; ++i)
    {
        data = &usdata[i].data;
        memcpy(data, &usdata[i].default_data, sizeof(*data));
    }
}

void
USInit(
        UserSettings *settings_init,
        SCSetting *items
        )
{
    int status;

    memset(settings_init, 0, sizeof(UserSettings));

    settings_init->cfg = SCParserCreate(UserSettingsLAST);
    settings_init->use_threads = 0;

    status = pthread_mutex_init(&settings_init->mutex, NULL);

    if(!status)
    {   settings_init->use_threads = 1;
    }

    if(settings_init->cfg)
    {
        settings_init->holder = items;
        USSetupCFGVars(settings_init);
        USSetupCFGDefaults(settings_init);
        USLoad(settings_init);
    }
}


void
USLoad(
        UserSettings *settings
        )
{
    US_LOCK_MUTEX(settings);

    if(!settings->cfg)
    {   goto UNLOCK;
    }

    SCParser *cfg = settings->cfg;
    SCItem *item;
    int status;
    char *configpath;

    configpath = (char *)WMConfigGetPath(WMFileConfig);

    if(!configpath)
    {   
        DebugWarn("Failed to get system config path, loading defaults");
        goto UNLOCK;
    }

    if(!FFFileExists(configpath))
    {
        US_UNLOCK_MUTEX(settings);
        USSave(settings);
        return;
    }

    status = SCParserReadFile(cfg, configpath);

    if(status)
    {   
        /* TODO: This sometimes prints, even when it shouldnt. FIXME */
        /*
         * if the file doesnt exist then we probably read the file when it was deleted
         * if(FFFileExists(configpath))
         * {   DebugWarn("Failed to load data?");
         * }
         */
        goto UNLOCK;
    }

    i32 i;
    SCSetting *setting;

    SCSetting *usdata = settings->holder;

    for(i = 0; i < UserSettingsLAST; ++i)
    {
        setting = &usdata[i];
        item = SCParserSearch(cfg, usdata[i].name);

        if(!item)
        {   item = SCParserSearchSlow(cfg, usdata[i].name);
        }

        if(item)
        {   
            #if DEBUG
                enum { SAFE_TYPE_BUFF_SIZE = 32 };

                char tmp[SAFE_TYPE_BUFF_SIZE];

                memcpy(tmp, &setting->data, MIN(sizeof(setting->data), SAFE_TYPE_BUFF_SIZE));
            #endif

            Generic prev = setting->data;
            Generic dsafe;

            status = SCParserLoad(item, &setting->data, SCParserGetTypeSize(usdata[i].type), usdata[i].type);

            dsafe = setting->data;

            #if DEBUG
                if(memcmp(&setting->data, tmp, MIN(sizeof(setting->data), SAFE_TYPE_BUFF_SIZE)))
                {   Debug("Updated: [%s]", usdata[i].name);
                }
            #endif

            if(status)
            {   
                if(!memcmp(&usdata[i].data, &usdata[i].default_data, sizeof(usdata[i].data)))
                {   DebugWarn("\"%s\" NOT FOUND, check your config for it.", usdata[i].name);
                }
                else
                {   DebugWarn("Failed to LOAD, \"%s\"", usdata[i].name);
                }
            }
            else
            {   
                if(setting->update_func)
                {   
                    US_UNLOCK_MUTEX(settings);
                    setting->update_func(prev, dsafe);
                    US_LOCK_MUTEX(settings);
                }
            }
        }
        else
        {   DebugWarn("Failed to FIND, \"%s\"", usdata[i].name);
        }
    }

UNLOCK:
    US_UNLOCK_MUTEX(settings);
}

void
USSave(
        UserSettings *settings
        )
{
    US_LOCK_MUTEX(settings);

    if(!settings->cfg)
    {   goto UNLOCK;
    }

    SCParser *cfg = settings->cfg;
    i32 i;

    SCSetting *usdata = settings->holder;

    for(i = 0; i < UserSettingsLAST; ++i)
    {   SCParserSaveVar(cfg, usdata[i].name, &usdata[i].data);
    }

    char *configpath;
    u8 status;

    configpath = (char *)WMConfigGetPath(WMFileConfig);

    if(!configpath)
    {   
        DebugWarn("Failed to get system config path, cannot save settings.");
        goto UNLOCK;
    }

    if(!FFFileExists(configpath))
    {
        status = FFCreateFile(configpath);

        if(status == EXIT_FAILURE)
        {   DebugWarn("Failed to create file, unable to write base config.");
        }
        else
        {
            Debug0("No file found, writing base config...");
            SCParserWrite(cfg, configpath);
        }
    }
    else if(FFIsFileEmpty(configpath))
    {
        Debug0("Empty file found, writing base config...");
        SCParserWrite(cfg, configpath);
    }
UNLOCK:
    US_UNLOCK_MUTEX(settings);
}

Generic
USDefaultSetting(
        UserSettings *settings,
        unsigned int setting
        )
{
    Generic ret;

    US_LOCK_MUTEX(settings);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].default_data;

    US_UNLOCK_MUTEX(settings);

    return ret;
}

Generic
USGetSetting(
        UserSettings *settings,
        unsigned int setting
        )
{
    Generic ret;

    US_LOCK_MUTEX(settings);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].data;

    US_UNLOCK_MUTEX(settings);

    return ret;
}

void
USSetSetting(
        UserSettings *settings,
        unsigned int setting,
        Generic data
        )
{
    US_LOCK_MUTEX(settings);

    SCSetting *usdata = settings->holder;

    usdata[setting].data = data;

    US_UNLOCK_MUTEX(settings);
}

void
USWipe(
        UserSettings *settings
        )
{
    if(!settings->cfg)
    {   return;
    }

    SCParser *cfg = settings->cfg;

    SCParserDestroy(cfg);
    pthread_mutex_destroy(&settings->mutex);
    memset(settings, 0, sizeof(UserSettings));
}
