#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "settings.h"
#include "util.h"

#define CONFIG_FILE             "/var/tmp/dwm-config"   /* todo make dir .config/dwm/config or someting like that */

#define _USER_SETTINGS_HOVER_FOCUS                  ((1 << 0))
#define _USER_SETTINGS_ENABLE_SERVER_DECORATIONS    ((1 << 1))
#define _USER_SETTINGS_ENABLE_CLIENT_DECORATIONS    ((1 << 2))
#define _USER_SETTINGS_PREFER_CLIENT_DECORATIONS    ((1 << 3))
#define _USER_SETTINGS_C        ((1 << 4))
#define _USER_SETTINGS_D        ((1 << 5))
#define _USER_SETTINGS_E        ((1 << 6))
#define _USER_SETTINGS_F        ((1 << 7))
#define _USER_SETTINGS_G        ((1 << 8))
#define _USER_SETTINGS_H        ((1 << 9))
#define _USER_SETTINGS_I        ((1 << 10))
#define _USER_SETTINGS_J        ((1 << 11))
#define _USER_SETTINGS_K        ((1 << 12))
#define _USER_SETTINGS_L        ((1 << 13))
#define _USER_SETTINGS_M        ((1 << 14))
#define _USER_SETTINGS_N        ((1 << 15))


/* helper */
#ifndef FIELD_SIZEOF
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#endif

enum
{
    Mfact,
    GapRatio,
    MCount,
    Snap,
    RefreshRate,
    Flags,
    MaxCC,


    BarLX,
    BarLY,
    BarLW,
    BarLH,

    BarRX,
    BarRY,
    BarRW,
    BarRH,

    BarTX,
    BarTY,
    BarTW,
    BarTH,

    BarBX,
    BarBY,
    BarBW,
    BarBH,

    UserSettingsLAST,
};


static char *const _DATA_ENTRY_NAME[UserSettingsLAST] = 
{
    [Mfact] = "Mfact",
    [GapRatio] = "GapRatio",
    [MCount] = "MCount",
    [Snap] = "Snap",
    [RefreshRate] = "RefreshRate",
    [Flags] = "Flags",
    [MaxCC] = "MaxCC",

    [BarLX] = "BarLX",
    [BarLY] = "BarLY",
    [BarLW] = "BarLW",
    [BarLH] = "BarLH",

    [BarRX] = "BarRX",
    [BarRY] = "BarRY",
    [BarRW] = "BarRW",
    [BarRH] = "BarRH",

    [BarTX] = "BarTX",
    [BarTY] = "BarTY",
    [BarTW] = "BarTW",
    [BarTH] = "BarTH",

    [BarBX] = "BarBX",
    [BarBY] = "BarBY",
    [BarBW] = "BarBW",
    [BarBH] = "BarBH",
};

static unsigned int _DATA_ENTRY_NAME_LEN[UserSettingsLAST] = 
{
    /* make all of these sizeof */
    [Mfact] = sizeof("Mfact"),
    [GapRatio] = sizeof("GapRatio"),
    [MCount] = sizeof("MCount"),
    [Snap] = sizeof("Snap"),
    [RefreshRate] = sizeof("RefreshRate"),
    [Flags] = sizeof("Flags"),
    [MaxCC] = sizeof("MaxCC"),

    [BarLX] = sizeof("BarLX"),
    [BarLY] = sizeof("BarLY"),
    [BarLW] = sizeof("BarLW"),
    [BarLH] = sizeof("BarLH"),

    [BarRX] = sizeof("BarRX"),
    [BarRY] = sizeof("BarRY"),
    [BarRW] = sizeof("BarRW"),
    [BarRH] = sizeof("BarRH"),

    [BarTX] = sizeof("BarTX"),
    [BarTY] = sizeof("BarTY"),
    [BarTW] = sizeof("BarTW"),
    [BarTH] = sizeof("BarTH"),

    [BarBX] = sizeof("BarBX"),
    [BarBY] = sizeof("BarBY"),
    [BarBW] = sizeof("BarBW"),
    [BarBH] = sizeof("BarBH")
};

static const unsigned int _DATA_ENTRY_SIZE[UserSettingsLAST] = 
{
    [Mfact] = FIELD_SIZEOF(UserSettings, mfact),
    [GapRatio] = FIELD_SIZEOF(UserSettings, gapratio),
    [MCount] = FIELD_SIZEOF(UserSettings, mcount),
    [Snap] = FIELD_SIZEOF(UserSettings, snap),
    [RefreshRate] = FIELD_SIZEOF(UserSettings, refreshrate),
    [Flags] = FIELD_SIZEOF(UserSettings, flags),
    [MaxCC] = FIELD_SIZEOF(UserSettings, maxcc),

    /* TODO */
    [BarLX] = FIELD_SIZEOF(BarSettings, lx),
    [BarLY] = FIELD_SIZEOF(BarSettings, ly),
    [BarLW] = FIELD_SIZEOF(BarSettings, lw),
    [BarLH] = FIELD_SIZEOF(BarSettings, lh),

    [BarRX] = FIELD_SIZEOF(BarSettings, rx),
    [BarRY] = FIELD_SIZEOF(BarSettings, ry),
    [BarRW] = FIELD_SIZEOF(BarSettings, rw),
    [BarRH] = FIELD_SIZEOF(BarSettings, rh),

    [BarTX] = FIELD_SIZEOF(BarSettings, tx),
    [BarTY] = FIELD_SIZEOF(BarSettings, ty),
    [BarTW] = FIELD_SIZEOF(BarSettings, tw),
    [BarTH] = FIELD_SIZEOF(BarSettings, th),

    [BarBX] = FIELD_SIZEOF(BarSettings, bx),
    [BarBY] = FIELD_SIZEOF(BarSettings, by),
    [BarBW] = FIELD_SIZEOF(BarSettings, bw),
    [BarBH] = FIELD_SIZEOF(BarSettings, bh),

};

#define __BAR_OFFSET  (offsetof(UserSettings, bar))
#define __BAR_OFFSET_INNER(NAME)  (offsetof(BarSettings, NAME))

#define BAR_OFFSET(ITEM_NAME) (__BAR_OFFSET + (__BAR_OFFSET_INNER(ITEM_NAME)))

static const unsigned int _DATA_ENTRY_OFFSET[UserSettingsLAST] = 
{
    [Mfact] = offsetof(UserSettings, mfact),
    [GapRatio] = offsetof(UserSettings, gapratio),
    [MCount] = offsetof(UserSettings, mcount),
    [Snap] = offsetof(UserSettings, snap),
    [RefreshRate] = offsetof(UserSettings, refreshrate),
    [Flags] = offsetof(UserSettings, flags),
    [MaxCC] = offsetof(UserSettings, maxcc),

    /* TODO */
    [BarLX] = BAR_OFFSET(lx),
    [BarLY] = BAR_OFFSET(ly),
    [BarLW] = BAR_OFFSET(lw),
    [BarLH] = BAR_OFFSET(lh),

    [BarRX] = BAR_OFFSET(rx),
    [BarRY] = BAR_OFFSET(ry),
    [BarRW] = BAR_OFFSET(rw),
    [BarRH] = BAR_OFFSET(rh),

    [BarTX] = BAR_OFFSET(tx),
    [BarTY] = BAR_OFFSET(ty),
    [BarTW] = BAR_OFFSET(tw),
    [BarTH] = BAR_OFFSET(th),

    [BarBX] = BAR_OFFSET(bx),
    [BarBY] = BAR_OFFSET(by),
    [BarBW] = BAR_OFFSET(bw),
    [BarBH] = BAR_OFFSET(bh),
};

static const enum SCType _DATA_ENTRY_TYPE[UserSettingsLAST] = 
{
    [Mfact] = SCTypeFLOAT,
    [GapRatio] = SCTypeFLOAT,
    [MCount] = SCTypeUSHORT,
    [Snap] = SCTypeUSHORT,
    [RefreshRate] = SCTypeUSHORT,
    [Flags] = SCTypeUSHORT,
    [MaxCC] = SCTypeUSHORT,

    /* TODO */
    [BarLX] = SCTypeFLOAT,
    [BarLY] = SCTypeFLOAT,
    [BarLW] = SCTypeFLOAT,
    [BarLH] = SCTypeFLOAT,

    [BarRX] = SCTypeFLOAT,
    [BarRY] = SCTypeFLOAT,
    [BarRW] = SCTypeFLOAT,
    [BarRH] = SCTypeFLOAT,

    [BarTX] = SCTypeFLOAT,
    [BarTY] = SCTypeFLOAT,
    [BarTW] = SCTypeFLOAT,
    [BarTH] = SCTypeFLOAT,

    [BarBX] = SCTypeFLOAT,
    [BarBY] = SCTypeFLOAT,
    [BarBW] = SCTypeFLOAT,
    [BarBH] = SCTypeFLOAT
};

void
USSetupCFGVars(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    SCParser *cfg = us->cfg;

    i32 i;
    u8 err = 0;
    const int READONLY = 1;
    for(i = 0; i < UserSettingsLAST; ++i)
    {   
        err = SCParserNewVar(cfg, _DATA_ENTRY_NAME[i], _DATA_ENTRY_NAME_LEN[i], READONLY, _DATA_ENTRY_SIZE[i], _DATA_ENTRY_TYPE[i]);
        if(err)
        {   DEBUG("Failed to create: \"%s\"", _DATA_ENTRY_NAME[i]);
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

    /* Do note these settings are mostly arbitrary numbers that I (the creator) like */
    UserSettings *s = us;
    const u16 nmaster = 1;

    const u8 hoverfocus = 0;            /* bool */
    const u8 serverdecor = 1;           /* bool */
    const u8 clientdecor = 1;           /* bool */
    const u8 preferclientdecor = 1;     /* bool */

    const u16 refreshrate = 60;
    const float bgw = 0.95f;
    const u16 winsnap = 10;
    const u16 maxcc = 256;      /* Xorg default is 255 or 256, I dont remember */
    const float mfact = 0.55f;

    s->mcount = nmaster;
    s->mfact = mfact;
    s->refreshrate = refreshrate;
    s->gapratio = bgw;
    s->snap = winsnap;
    s->maxcc = maxcc;

    /* flags */
    SETFLAG(s->flags, _USER_SETTINGS_HOVER_FOCUS, hoverfocus);
    SETFLAG(s->flags, _USER_SETTINGS_ENABLE_SERVER_DECORATIONS, serverdecor);
    SETFLAG(s->flags, _USER_SETTINGS_ENABLE_CLIENT_DECORATIONS, clientdecor);
    SETFLAG(s->flags, _USER_SETTINGS_PREFER_CLIENT_DECORATIONS, preferclientdecor);

    BarSettings *bs = &us->bar;
    /* Left Stuff */
    bs->lw = 0.15f;
    bs->lh = 1.0f;
    bs->lx = 0.0f;
    bs->ly = 0.0f;
    /* Right Stuff */
    bs->rw = 0.15f;
    bs->rh = 1.0f;
    bs->rx = 1.0f - bs->rw;
    bs->ry = 0.0f;
    /* Top Stuff */
    bs->tw = 1.0f;
    bs->th = 0.15f;
    bs->tx = 0.0f;
    bs->ty = 0.0f;
    /* Bottom Stuff */
    bs->bw = 1.0f;
    bs->bh = 0.15f;
    bs->bx = 0.0f;
    bs->by = 1.0f - bs->bh;
}

void
USInit(
        UserSettings *settings_init
        )
{
    if(!settings_init)
    {   return;
    }
    memset(settings_init, 0, sizeof(UserSettings));
    settings_init->cfg = SCPParserCreate(UserSettingsLAST);
    if(settings_init->cfg)
    {
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
    if(!settings || !settings->cfg)
    {   return;
    }

    SCParser *cfg = settings->cfg;
    SCItem *item;
    u8 status = SCParserReadFile(cfg, CONFIG_FILE);
    if(status)
    {   
        DEBUG0("Failed to load previous data, loading defaults...");
        return;
    }
    i32 i;
    void *data;
    for(i = 0; i < UserSettingsLAST; ++i)
    {
        data = ((uint8_t *)settings) + _DATA_ENTRY_OFFSET[i];
        item = SCParserSearch(cfg, _DATA_ENTRY_NAME[i]);
        if(!item)
        {   item = SCParserSearchSlow(cfg, _DATA_ENTRY_NAME[i]);
        }
        if(item)
        {   SCParserLoad(item, data, _DATA_ENTRY_SIZE[i], _DATA_ENTRY_TYPE[i]);
        }
        else
        {   DEBUG("Failed to load, \"%s\"", _DATA_ENTRY_NAME[i]);
        }
    }
}

void
USSave(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }
    SCParser *cfg = settings->cfg;
    UserSettings *s = settings;
    i32 i;
    for(i = 0; i < UserSettingsLAST; ++i)
    {   SCParserSaveVar(cfg, _DATA_ENTRY_NAME[i], ((uint8_t *)s) + _DATA_ENTRY_OFFSET[i]);
    }
    SCParserWrite(cfg, CONFIG_FILE);
}

void
USWipe(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }
    SCParser *cfg = settings->cfg;
    SCParserDestroy(cfg);
    memset(settings, 0, sizeof(UserSettings));
}

