#ifndef __WM__CONFIG__H__
#define __WM__CONFIG__H__

#include "usersettings.h"

#include <stdint.h>

enum
WMFiles
{
    WMFileConfig,
    WMFileSession,
    WMFileStartup,
    WMFileLua,
    WMFileFolder,
    WMFileLAST
};

int
WMConfigInit(
        void
        );

void
WMConfigDestroy(
        void
        );

const char *
WMConfigGetPath(
        enum WMFiles file
        );

#endif
