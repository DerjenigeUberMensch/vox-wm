#ifndef __WM__MANIFEST__H__
#define __WM__MANIFEST__H__

#include "util.h"

#define MANIFEST_VERSION_UNKNOWN 0
#define MANIFEST_DATABASE_VERSION_UNKNOWN 0

typedef struct WMManifest WMManifest;

struct
WMManifest
{
    int64_t version;
    int64_t databaseVersion;
};

int InitManifest(void);
void DestroyManifest(void);
const WMManifest *GetManifest(void);

#endif