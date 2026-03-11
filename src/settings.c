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
        {   Debug("Failed to create: \"%s\"", usdata[i].name);
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
    pthread_mutex_lock(&settings->mutex);

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
        Debug0("Failed to get system config path, loading defaults");
        goto UNLOCK;
    }

    if(!FFFileExists(configpath))
    {
        pthread_mutex_unlock(&settings->mutex);
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
         * {   Debug0("Failed to load data?");
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
            {   Debug("Failed to LOAD, \"%s\"", usdata[i].name);
            }
            else
            {   
                if(setting->update_func)
                {   
                    pthread_mutex_unlock(&settings->mutex);
                    setting->update_func(prev, dsafe);
                    pthread_mutex_lock(&settings->mutex);
                }
            }
        }
        else
        {   Debug("Failed to FIND, \"%s\"", usdata[i].name);
        }
    }

UNLOCK:
    pthread_mutex_unlock(&settings->mutex);
}

void
USSave(
        UserSettings *settings
        )
{
    pthread_mutex_lock(&settings->mutex);
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
        Debug0("Failed to get system config path, cannot save settings.");
        goto UNLOCK;
    }

    if(!FFFileExists(configpath))
    {
        status = FFCreateFile(configpath);

        if(status == EXIT_FAILURE)
        {   Debug0("Failed to create file, unable to write base config.");
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
    pthread_mutex_unlock(&settings->mutex);
}

Generic
USDefaultSetting(
        UserSettings *settings,
        unsigned int setting
        )
{
    Generic ret;

    pthread_mutex_lock(&settings->mutex);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].default_data;

    pthread_mutex_unlock(&settings->mutex);

    return ret;
}

Generic
USGetSetting(
        UserSettings *settings,
        unsigned int setting
        )
{
    Generic ret;

    pthread_mutex_lock(&settings->mutex);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].data;

    pthread_mutex_unlock(&settings->mutex);

    return ret;
}

void
USSetSetting(
        UserSettings *settings,
        unsigned int setting,
        Generic data
        )
{
    pthread_mutex_lock(&settings->mutex);

    SCSetting *usdata = settings->holder;

    usdata[setting].data = data;

    pthread_mutex_unlock(&settings->mutex);
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
