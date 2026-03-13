#include <wordexp.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#include "config.h"
#include "toggle.h"
#include "util.h"
#include "file_util.h"

#include "startupapps.h"


void
ReadStartupApps(
        void
        )
{
    enum { ERROR = -1 };
    enum { OVERFLOW_RETURN = -1 };
    enum { PARSER_ERROR = -2 };
    enum { SUCCESS = 0 };
    enum { EOF_RETURN = 1 };

    int status;
    char *config;

    config = (char *)WMConfigGetPath(WMFileStartup);

    if(!config)
    {   
        DebugI("%s", "Failed to load Startup Apps");
        return;
    }

    if(!FFFileExists(config))
    {   
        status = FFCreateFile(config);

        if(unlikely(status == EXIT_FAILURE))
        {   Debug0("Failed to create empty file for startup config.");
        }
        else
        {   Debug("Created Startup Config! -> %s", config);
        }

        return;
    }

    FILE *fr = fopen(config, "r");
    
    int fd;

    if(!fr)
    {   return;
    }

    fd = fileno(fr);

    if(fd == ERROR)
    {   
        perror("fileno");
        fclose(fr);
        return;
    }

    status = FFLockFileRead(fd, true);

    if(status == ERROR)
    {   
        fclose(fr);
        return;
    }

    enum { MAX_LENGTH = 1024 };

    char buff[MAX_LENGTH];
    wordexp_t word;
    Arg arg = { .v = NULL };
    char *olddir = NULL;

    memset(buff, 0, sizeof(buff));

    olddir = getcwd(NULL, 0);

    /* some apps run from their root dir which would be in .config/vox-wm/startup.cfg or whaever. */
    if(olddir)
    {   
        const char *wmfolder = WMConfigGetPath(WMFileFolder);

        chdir(wmfolder);
    }

    while(true)
    {
        status = FFGetNewLine(fr, buff, MAX_LENGTH - 1);

        if(status == EOF_RETURN)
        {   break;
        }

        if(status == OVERFLOW_RETURN)
        {   
            Debug0("Overflowed bufer when trying to load startup app");
            continue;
        }
        else if(status == PARSER_ERROR)
        {   
            Debug0("Reached Parser error while trying to load statup app");
            continue;
        }

        if(!ASSERT(status == SUCCESS))
        {   
            Debug0("Reached invalid state");
            continue;
        }

        if(wordexp(buff, &word, WRDE_NOCMD) != 0)
        {   
            Debug("Failed to parse %s", buff);
            continue;
        }

        arg.v = word.we_wordv;

    
        SpawnWindow(&arg);

        wordfree(&word);
    }

    if(olddir)
    {   chdir(olddir);
    }

    free(olddir);

    FFUnlockFileRead(fd);
}
