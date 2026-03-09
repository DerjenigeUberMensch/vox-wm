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
        err = SCParserNewVar(cfg, usdata->name, usdata->name_len, READONLY, usdata->size, usdata->type);
        if(err)
        {   Debug("Failed to create: \"%s\"", usdata->name);
        }
        ++usdata;
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

    UserSettings *s = us;
    void *data;
    i32 i;

    const SCSetting *usdata = us->holder;

    for(i = 0; i < UserSettingsLAST; ++i)
    {
        data = ((uint8_t *)s) + usdata->offset;
        memcpy(data, &usdata->default_data, usdata->size);
        ++usdata;
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
    {   
        settings_init->use_threads = 1;
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

    char __CONFIG__PATH__[FFSysGetConfigPathLengthMAX];

    SCParser *cfg = settings->cfg;
    SCItem *item;
    u8 status;

    status = WMConfigGetSettingsPath(__CONFIG__PATH__, FFSysGetConfigPathLengthMAX, NULL);

    if(status)
    {   
        Debug0("Failed to get system config path, loading defaults");
        goto UNLOCK;
    }

    status = SCParserReadFile(cfg, __CONFIG__PATH__);

    if(status)
    {   
        /* TODO: This sometimes prints, even when it shouldnt. FIXME */
        /*
         * if the file doesnt exist then we probably read the file when it was deleted
         * if(FFFileExists(__CONFIG__PATH__))
         * {   Debug0("Failed to load data?");
         * }
         */
        goto UNLOCK;
    }

    i32 i;
    SCSetting *data;

    SCSetting *usdata = settings->holder;

    for(i = 0; i < UserSettingsLAST; ++i)
    {
        data = &usdata[i];
        item = SCParserSearch(cfg, usdata->name);

        if(!item)
        {   item = SCParserSearchSlow(cfg, usdata->name);
        }

        if(item)
        {   
            #if DEBUG
                enum { SAFE_TYPE_BUFF_SIZE = 32 };

                char tmp[SAFE_TYPE_BUFF_SIZE];

                memcpy(tmp, data, MIN(usdata->size, SAFE_TYPE_BUFF_SIZE));
            #endif

            Generic prev = data->data;

            status = SCParserLoad(item, data, usdata->size, usdata->type);

            #if DEBUG
                if(memcmp(data, tmp, MIN(usdata->size, SAFE_TYPE_BUFF_SIZE)))
                {   Debug("Updated: [%s]", usdata->name);
                }
            #endif

            if(status)
            {   Debug("Failed to LOAD, \"%s\"", usdata->name);
            }
            else
            {   data->update_func(prev, data->data);
            }
        }
        else
        {   Debug("Failed to FIND, \"%s\"", usdata->name);
        }

        ++usdata;
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
    UserSettings *s = settings;
    i32 i;

    const SCSetting *usdata = us->settings;

    for(i = 0; i < UserSettingsLAST; ++i)
    {   
        SCParserSaveVar(cfg, usdata->name, ((uint8_t *)usdata) + usdata->offset);
        ++usdata;
    }

    char __CONFIG__PATH__[FFSysGetConfigPathLengthMAX];
    u8 status = WMConfigGetSettingsPath(__CONFIG__PATH__, FFSysGetConfigPathLengthMAX, NULL);

    if(status)
    {   
        Debug0("Failed to get system config path, cannot save settings.");
        goto UNLOCK;
    }

    if(!FFFileExists(__CONFIG__PATH__))
    {
        status = FFCreateFile(__CONFIG__PATH__);
        if(status == EXIT_FAILURE)
        {   Debug0("Failed to create file, unable to write base config.");
        }
        else
        {
            Debug0("No file found, writing base config...");
            SCParserWrite(cfg, __CONFIG__PATH__);
        }
    }
    else if(FFIsFileEmpty(__CONFIG__PATH__))
    {
        Debug0("Empty file found, writing base config...");
        SCParserWrite(cfg, __CONFIG__PATH__);
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

    pthread_mutex_lock(&setting->mutex);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].default_data;

    pthread_mutex_unlock(&setting->mutex);

    return ret;
}

Generic
USGetSetting(
        UserSettings *settings,
        unsigned int setting
        )
{
    Generic ret;

    pthread_mutex_lock(&setting->mutex);

    const SCSetting *usdata = settings->holder;

    ret = usdata[setting].data;

    pthread_mutex_unlock(&setting->mutex);

    return ret;
}

void
USSetSetting(
        UserSettings *settings,
        unsigned int setting,
        Generic data
        )
{
    pthread_mutex_lock(&setting->mutex);

    const SCSetting *usdata = settings->holder;

    usdata[setting].data = data;

    pthread_mutex_unlock(&setting->mutex);
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
