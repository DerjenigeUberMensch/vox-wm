#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "SCParser/parser.h"

#include "config.h"
#include "file_util.h"
#include "settings.h"
#include "manifest.h"

#define MANIFEST_VERSION 1
#define DATABASE_VERSION 1

static WMManifest manifest;
static int initialized = 0;

static const char *const MANIFEST_KEY = "manifest";
static const char *const MANIFEST_DATABASE_KEY = "database";

static int 
INIT_MANIFEST_v1(const char *manifestPath)
{
    enum { NUM_VARS = 2 };

    SCParser *parser = SCParserCreate(NUM_VARS);

    if(!parser)
    {   return EXIT_FAILURE;
    }

    int status;
    int ret = EXIT_FAILURE;

    status = SCParserNewVar(parser, MANIFEST_KEY, strlen(MANIFEST_KEY) + 1, 1, 4, SCTypeLONG);

    if(status)
    {
        ret = EXIT_FAILURE;
        goto FREE;
    }

    status = SCParserNewVar(parser, MANIFEST_DATABASE_KEY, strlen(MANIFEST_DATABASE_KEY) + 1, 1, 4, SCTypeLONG);

    if(status)
    {
        ret = EXIT_FAILURE;
        goto FREE;
    }

    SCItem *itemmanifest = SCParserSearch(parser, MANIFEST_KEY);

    if(!itemmanifest)
    {
        itemmanifest = SCParserSearchSlow(parser, MANIFEST_KEY);

        if(!itemmanifest)
        {
            ret = EXIT_FAILURE;
            goto FREE;
        }
    }

    SCItem *itemdatabase = SCParserSearch(parser, MANIFEST_DATABASE_KEY);

    if(!itemdatabase)
    {
        itemdatabase = SCParserSearchSlow(parser, MANIFEST_DATABASE_KEY);

        if(!itemdatabase)
        {
            ret = EXIT_FAILURE;
            goto FREE;
        }
    }

    /* does it exist? */
    if(!FFFileExists(manifestPath))
    {   
        ret = EXIT_FAILURE;
        goto FREE;
    }

    if(FFIsFileEmpty(manifestPath))
    {
        i64 currentManifest = MANIFEST_VERSION;
        i64 currentDatabase = DATABASE_VERSION;

        status = SCParserSaveVar(parser, MANIFEST_KEY, &currentManifest);

        if(status)
        {
            ret = EXIT_FAILURE;
            goto FREE;
        }

        status = SCParserSaveVar(parser, MANIFEST_DATABASE_KEY, &currentDatabase);

        if(status)
        {
            ret = EXIT_FAILURE;
            goto FREE;
        }

        if(!FFFileExists(manifestPath))
        {   
            DebugWarn("Failed to create or initialize manifest file");
            ret = EXIT_FAILURE;
            goto FREE;
        }
        else
        {
            if(FFIsFileEmpty(manifestPath))
            {   
                status = SCParserWrite(parser, manifestPath);

                if(status)
                {
                    ret = EXIT_FAILURE;
                    goto FREE;
                }

                DebugLog("Loaded new manifest file");

                manifest.version = MANIFEST_VERSION;
                manifest.databaseVersion = DATABASE_VERSION;

                ret = EXIT_SUCCESS;

                goto FREE;
            }
            else
            {   DebugWarn("Manifest file is not empty");
            }
        }
    }

    status = SCParserReadFile(parser, manifestPath);

    if(status)
    {
        ret = EXIT_FAILURE;
        goto FREE;
    }

    i64 manifestVersion = 0;
    i64 databaseVersion = 0;

    status = SCParserLoad(itemmanifest, &manifestVersion, SCParserGetTypeSize(SCTypeLONG), SCTypeLONG);

    if(status)
    {
        ret = EXIT_FAILURE;
        goto FREE;
    }

    status = SCParserLoad(itemdatabase, &databaseVersion, SCParserGetTypeSize(SCTypeLONG), SCTypeLONG);

    if(status)
    {
        ret = EXIT_FAILURE;
        goto FREE;
    }

    manifest.version = manifestVersion;
    manifest.databaseVersion = databaseVersion;

    ret = EXIT_SUCCESS;
FREE:

    SCParserDestroy(parser);

    return ret;
}

int
InitManifest(void)
{
    if(initialized)
    {   return EXIT_SUCCESS;
    }

    const char *manifestPath = WMConfigGetPath(WMFileManifest);

    if(unlikely(!manifestPath))
    {   
        DebugWarn("Failed to get manifest path");
        return EXIT_FAILURE;
    }

    int status;

    if(!FFFileExists(manifestPath))
    {
        status = FFCreateFile((char *)manifestPath);

        if(status == EXIT_FAILURE)
        {
            DebugWarn("Failed to create manifest file");
            return EXIT_FAILURE;
        }
    }

    status = INIT_MANIFEST_v1(manifestPath);

    if(status == EXIT_FAILURE)
    {
        DebugWarn("Failed to initialize manifest");
        return EXIT_FAILURE;
    }

    initialized = 1;

    return EXIT_SUCCESS;
}

void 
DestroyManifest(void)
{
    if(initialized)
    {   
        memset(&manifest, 0, sizeof(WMManifest));
        initialized = 0;
    }
}

const WMManifest *
GetManifest(void)
{   
    if(!initialized)
    {   return NULL;
    }

    return &manifest;
}
