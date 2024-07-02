
#include <stdint.h>
#include "settings.h"
#include "parser.h"
#include "util.h"
#include "desktop.h"
#include <string.h>



void
USSetupCFGVars(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    CFG *cfg = us->cfg;
    CFGCreateVar(cfg, "MCount", USHORT);
    CFGCreateVar(cfg, "Layout", UCHAR);
    CFGCreateVar(cfg, "OLayout", UCHAR);
    CFGCreateVar(cfg, "DefaultDesk", USHORT);
    CFGCreateVar(cfg, "HoverFocus", UCHAR);
    CFGCreateVar(cfg, "RefreshRate", USHORT);
    CFGCreateVar(cfg, "GapRatio", FLOAT);
    CFGCreateVar(cfg, "Snap", USHORT);
    CFGCreateVar(cfg, "MaxClientCount", USHORT);
    CFGCreateVar(cfg, "MFact", FLOAT);
    /* Left */
    CFGCreateVar(cfg, "BarLW", FLOAT);
    CFGCreateVar(cfg, "BarLH", FLOAT);
    CFGCreateVar(cfg, "BarLX", FLOAT);
    CFGCreateVar(cfg, "BarLY", FLOAT);
    /* Right */
    CFGCreateVar(cfg, "BarRW", FLOAT);
    CFGCreateVar(cfg, "BarRH", FLOAT);
    CFGCreateVar(cfg, "BarRX", FLOAT);
    CFGCreateVar(cfg, "BarRY", FLOAT);
    /* Top */
    CFGCreateVar(cfg, "BarTW", FLOAT);
    CFGCreateVar(cfg, "BarTH", FLOAT);
    CFGCreateVar(cfg, "BarTX", FLOAT);
    CFGCreateVar(cfg, "BarTY", FLOAT);
    /* Bottom */
    CFGCreateVar(cfg, "BarBW", FLOAT);
    CFGCreateVar(cfg, "BarBH", FLOAT);
    CFGCreateVar(cfg, "BarBX", FLOAT);
    CFGCreateVar(cfg, "BarBY", FLOAT);
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
    const u8 hoverfocus = 0;   /* bool */
    const u8 desktoplayout = Monocle;
    const u8 odesktoplayout = Tiled;
    const u16 defaultdesktop = 0;
    const u16 refreshrate = 60;
    const float bgw = 0.95f;
    const u16 winsnap = 10;
    const u16 maxcc = 256;      /* Xorg default is 255 or 256, I dont remember */
    const float mfact = 0.55f;

    USSetMCount(s, nmaster);
    USSetLayout(s, desktoplayout);
    USSetOLayout(s, odesktoplayout);
    USSetDefaultDesk(s, defaultdesktop);
    USSetHoverFocus(s, hoverfocus);
    USSetRefreshRate(s, refreshrate);
    USSetGapRatio(s, bgw);
    USSetSnap(s, winsnap);
    USSetMaxClientCount(s, maxcc);
    USSetMFact(s, mfact);

    BarSettings *bs = USGetBarSettings(us);
    /* Left Stuff */
    bs->left.w = .15f;
    bs->left.h = 1.0f;
    bs->left.x = 0.0f;
    bs->left.y = 0.0f;
    /* Right Stuff */
    bs->right.w = .15f;
    bs->right.h = 1.0f;
    bs->right.x = 1.0f - bs->right.w;
    bs->right.y = 0.0f;
    /* Top Stuff */
    bs->top.w = 1.0f;
    bs->top.h = .15f;
    bs->top.x = 0.0f;
    bs->top.y = 0.0f;
    /* Bottom Stuff */
    bs->bottom.w = 1.0f;
    bs->bottom.h = .15f;
    bs->bottom.x = 0.0f;
    bs->bottom.y = 1.0f - bs->bottom.h;
}

void
USInit(
        UserSettings *settings_init,
        char *CONFIG_FILE
        )
{
    if(!settings_init)
    {   return;
    }
    memset(settings_init, 0, sizeof(UserSettings));
    settings_init->cfg = CFGCreate(CONFIG_FILE);
    if(settings_init->cfg)
    {
        USSetupCFGVars(settings_init);
        USSetupCFGDefaults(settings_init);
        if(CFGLoad(settings_init->cfg))
        {   
            /* kinda a lie since we setup defaults before as a failsafe incase we get wack numbers, mostly just for debug purposes though */
            DEBUG0("Failed to load previous data, loading default settings...");
            USSave(settings_init);
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
    CFG *cfg = settings->cfg;
    UserSettings *s = settings;
    BarSettings b = settings->bar;
    int32_t hoverfocus = USIsHoverFocus(s);
    CFGSaveVar(cfg, "MCount", &s->mcount);
    CFGSaveVar(cfg, "Layout", &s->defaultlayout);
    CFGSaveVar(cfg, "OLayout", &s->odefaultlayout);
    CFGSaveVar(cfg, "DefaultDesk", &s->defaultdesk);
    CFGSaveVar(cfg, "HoverFocus", &hoverfocus);
    CFGSaveVar(cfg, "RefreshRate", &s->refreshrate);
    CFGSaveVar(cfg, "GapRatio", &s->gapratio);
    CFGSaveVar(cfg, "Snap", &s->snap);
    CFGSaveVar(cfg, "MaxClientCount", &s->maxcc);
    CFGSaveVar(cfg, "MFact", &s->mfact);
    /* Left */
    CFGSaveVar(cfg, "BarLW", &b.left.w);
    CFGSaveVar(cfg, "BarLH", &b.left.h);
    CFGSaveVar(cfg, "BarLX", &b.left.x);
    CFGSaveVar(cfg, "BarLY", &b.left.y);
    /* Right */
    CFGSaveVar(cfg, "BarRW", &b.right.w);
    CFGSaveVar(cfg, "BarRH", &b.right.h);
    CFGSaveVar(cfg, "BarRX", &b.right.x);
    CFGSaveVar(cfg, "BarRY", &b.right.y);
    /* Top */
    CFGSaveVar(cfg, "BarTW", &b.top.w);
    CFGSaveVar(cfg, "BarTH", &b.top.h);
    CFGSaveVar(cfg, "BarTX", &b.top.x);
    CFGSaveVar(cfg, "BarTY", &b.top.y);
    /* Bottom */
    CFGSaveVar(cfg, "BarBW", &b.bottom.w);
    CFGSaveVar(cfg, "BarBH", &b.bottom.h);
    CFGSaveVar(cfg, "BarBX", &b.bottom.x);
    CFGSaveVar(cfg, "BarBY", &b.bottom.y);
    CFGWrite(cfg);
}

void
USWipe(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }
    CFG *cfg = settings->cfg;
    CFGDestroy(cfg);
    memset(settings, 0, sizeof(UserSettings));
}

void
USSetHoverFocus(
        UserSettings *settings, 
        uint8_t state
        )
{
    SETFLAG(settings->flags, _USER_SETTINGS_HOVER_FOCUS, state);
}

uint8_t
USIsHoverFocus(
        UserSettings *settings
        )
{
    return !!FLAGSET(settings->flags, _USER_SETTINGS_HOVER_FOCUS);
}

void
USSetMFact(
        UserSettings *settings, 
        float mfact
        )
{
    settings->mfact = mfact;
}

float
USGetMFact(
        UserSettings *settings
        )
{
    return settings->mfact;
}

void
USSetMCount(
        UserSettings *settings,
        uint16_t mcount
        )
{
    settings->mcount = mcount;
}

uint16_t
USGetMCount(
        UserSettings *settings
        )
{
    return settings->mcount;
}

void
USSetLayout(
        UserSettings *settings,
        uint8_t layout
        )
{
    settings->defaultlayout = layout;
}

uint8_t
USGetLayout(
        UserSettings *settings
        )
{
    return settings->defaultlayout;
}

void
USSetOLayout(
        UserSettings *settings,
        uint8_t layout
        )
{
    settings->odefaultlayout = layout;
}

uint8_t
USGetOLayout(
        UserSettings *settings
        )
{
    return settings->odefaultlayout;
}

void
USSetDefaultDesk(
        UserSettings *settings,
        uint16_t desk 
        )
{
    settings->defaultdesk = desk;
}

uint16_t
USGetDefaultDesk(
        UserSettings *settings
        )
{
    return settings->defaultdesk;
}

void
USSetGapRatio(
        UserSettings *settings,
        float ratio
        )
{
    settings->gapratio = ratio;
}

float
USGetGapRatio(
        UserSettings *settings
        )
{
    return settings->gapratio;
}

void
USSetSnap(
        UserSettings *settings,
        uint16_t snap
        )
{
    settings->snap = snap;
}

uint16_t
USGetSnap(
        UserSettings *settings
        )
{
    return settings->snap;
}

void
USSetRefreshRate(
        UserSettings *settings,
        uint16_t rate
        )
{
    settings->refreshrate = rate;
}

uint16_t
USGetRefreshRate(
        UserSettings *settings
        )
{
    return settings->refreshrate;
}


void
USSetMaxClientCount(
        UserSettings *settings,
        uint16_t maxcc 
        )
{
    settings->maxcc = maxcc;
}

uint16_t
USGetMaxClientCount(
        UserSettings *settings
        )
{
    return settings->maxcc;
}

BarSettings *
USGetBarSettings(
        UserSettings *settings
        )
{
    return &settings->bar;
}
