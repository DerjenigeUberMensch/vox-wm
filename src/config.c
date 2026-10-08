#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "main.h"
#include "util.h"
#include "config.h"

typedef struct WMConfigPath WMConfigPath;
typedef struct WMConfigPathRegistry WMConfigPathRegistry;

struct
WMConfigPathRegistry
{
    enum WMFolder folder;
    const char *file;
};

static const WMConfigPathRegistry WM_CONFIG_PATHS_REGISTRY[WMFileLAST] =
{
    [WMFileConfig]   = { .folder = WMFolderConfig, .file = "vox.cfg" },
    [WMFileStartup]  = { .folder = WMFolderConfig, .file = "startup.cfg" },
    [WMFileLua]      = { .folder = WMFolderConfig, .file = "vox.lua" },
    [WMFileManifest] = { .folder = WMFolderData,   .file = "manifest.txt" },

    [WMFileSession]  =  { .folder = WMFolderData, .file = "session.cfg" },
};

static char *WM_FILES[WMFileLAST] = {0};
static char *WM_FOLDERS[WMFolderLAST] = {0};

static char *GENERATE_FILE_PATH(enum WMFolder folder, const char *file)
{
    (void)ASSERT(folder != WMFolderLAST);

    if(file == NULL)
    {   file = "";
    }
    
    enum { BUFF_LENGTH = MAX(FFSysGetConfigPathLengthMAX, FFSysGetDataPathLengthMAX) };

    size_t len;
    int status;
    char *ret = NULL;
    char *preappendpath = "/vox-wm/";
    char buffpath[BUFF_LENGTH + 1];

    memset(buffpath, 0, sizeof(buffpath));

    switch(folder)
    {
        case WMFolderConfig:
            status = FFGetSysConfigPath(buffpath, BUFF_LENGTH, &len);
            break;
        case WMFolderData:
        case WMFolderDataBase:
            status = FFGetSysDataPath(buffpath, BUFF_LENGTH, &len);
            break;
        default: status = EXIT_FAILURE; break;
    }

    if(status != EXIT_SUCCESS)
    {   return ret;
    }

    char *subfolder= "";

    if(folder == WMFolderDataBase)
    {   subfolder = "db/";
    }

    ret = strjoin(
        "%s" "%s"
            "%s" "%s", 
            buffpath, preappendpath, 
            subfolder, file
        );

    return ret;
}

/* TODO */
int
WMConfigInit(
        void
        )
{
    size_t configpathlen = FFGetSysConfigPathLength();
    size_t datapathlen = FFGetSysDataPathLength();

    if(configpathlen == 0 || datapathlen == 0)
    {   
        DebugWarn("Failed to get config path.");
        return EXIT_FAILURE;
    }

    int i;

    for(i = 0; i < WMFileLAST; ++i)
    {   WM_FILES[i] = GENERATE_FILE_PATH(WM_CONFIG_PATHS_REGISTRY[i].folder, WM_CONFIG_PATHS_REGISTRY[i].file);
    }

    for(i = 0; i < WMFolderLAST; ++i)
    {   WM_FOLDERS[i] = GENERATE_FILE_PATH((enum WMFolder)i, NULL);
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

    for(i = 0; i < WMFolderLAST; ++i)
    {   
        free(WM_FOLDERS[i]);
        WM_FOLDERS[i] = NULL;
    }
}

const char *
WMConfigGetPath(
        enum WMFile file
        )
{   return (const char *)WM_FILES[file];
}

const char *
WMConfigGetFolder(
        enum WMFolder folder
        )
{   return (const char *)WM_FOLDERS[folder];
}