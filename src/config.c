
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "config.h"

static const char *const WM_FOLDER = "/vox-wm/";
static const char *const WM_FILE_LIST[WMFileLAST] = 
{
    [WMFileConfig] = "vox.cfg",
    [WMFileSession] = "session.cfg",
    [WMFileStartup] = "startup.cfg",
    [WMFileFolder] = "",
};

static char *WM_FILES[WMFileLAST] = 
{
    [WMFileConfig] = NULL,
    [WMFileSession] = NULL,
};


int
WMConfigInit(
        void
        )
{
    size_t configpathlen = FFGetSysConfigPathLength();

    if(configpathlen == 0)
    {   
        DebugI("%s", "Failed to get config path.");
        return EXIT_FAILURE;
    }

    int i;
    u8 status;

    for(i = 0; i < WMFileLAST; ++i)
    {
        size_t wmfolderlen = strlen(WM_FOLDER);
        size_t wmfilelen = strlen(WM_FILE_LIST[i]);
        size_t size = sizeof(char) + sizeof(char) * (configpathlen + wmfolderlen + wmfilelen);
        size_t lencur = 0;

        WM_FILES[i] = malloc(size);

        if(!WM_FILES[i])
        {   continue;
        }

        status = FFGetSysConfigPath(WM_FILES[i], size, &lencur);

        if(unlikely(status == EXIT_FAILURE) || !ASSERT(lencur == configpathlen))
        {   
            free(WM_FILES[i]);
            WM_FILES[i] = NULL;
            continue;
        }

        memcpy(WM_FILES[i] + lencur, WM_FOLDER, wmfolderlen);
        lencur += wmfolderlen;
        memcpy(WM_FILES[i] + lencur, WM_FILE_LIST[i], wmfilelen);

        WM_FILES[i][size - 1] = '\0';
    }


    return EXIT_SUCCESS;
}

void
WMConfigDestroy(
        void
        )
{
    int i;

    for(i = 0; i < WMFileLAST; ++i)
    {   
        free(WM_FILES[i]);
        WM_FILES[i] = NULL;
    }
}

const char *
WMConfigGetPath(
        enum WMFiles file
        )
{   return (const char *)WM_FILES[file];
}
