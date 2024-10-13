
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "config.h"



int
WMConfigGetConfigPath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        )
{
    if(!buff || !buff_length)
    {   return EXIT_FAILURE;
    }
    enum { CONFIG_DIR_LENGTH_NAME = sizeof("vox-wm/") };
    const char *const CONFIG_DIR_NAME = "vox-wm/";

    unsigned int len = 0;
    int status;

    status = FFGetSysConfigPath(buff, buff_length, &len);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    if(len + CONFIG_DIR_LENGTH_NAME > buff_length)
    {   return EXIT_FAILURE;
    }

    memcpy(buff + len, CONFIG_DIR_NAME, CONFIG_DIR_LENGTH_NAME);
    len += CONFIG_DIR_LENGTH_NAME;

    if(len_return)
    {   *len_return = len;
    }
    return EXIT_SUCCESS;
}

int
WMConfigGetSessionPath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        )
{
    if(!buff || !buff_length)
    {   return EXIT_FAILURE;
    }

    enum { SESSION_FILE_NAME_LENGTH = sizeof("session.cfg") };
    const char *const SESSION_FILE_NAME = "session.cfg";

    int status;
    uint32_t len = 0;

    status = WMConfigGetConfigPath(buff, buff_length, &len);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }
    if(len + SESSION_FILE_NAME_LENGTH > buff_length)
    {   return EXIT_FAILURE;
    }
    memcpy(buff + len - 1, SESSION_FILE_NAME, SESSION_FILE_NAME_LENGTH);

    if(len_return)
    {   *len_return = len;
    }
    return EXIT_SUCCESS;
}

int
WMConfigGetSettingsPath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        )
{
    if(!buff || !buff_length)
    {   return EXIT_FAILURE;
    }

    enum { SETTINGS_FILE_NAME_LENGTH = sizeof("vox.cfg") };
    const char *const SETTINGS_FILE_NAME = "vox.cfg";

    int status;
    uint32_t len = 0;

    status = WMConfigGetConfigPath(buff, buff_length, &len);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }
    if(len + SETTINGS_FILE_NAME_LENGTH > buff_length)
    {   return EXIT_FAILURE;
    }
    memcpy(buff + len - 1, SETTINGS_FILE_NAME, SETTINGS_FILE_NAME_LENGTH);

    if(len_return)
    {   *len_return = len;
    }
    return EXIT_SUCCESS;
}
