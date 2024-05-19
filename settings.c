
#include <stdint.h>
#include "settings.h"
#include "util.h"



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
USSetGapWidth(
        UserSettings *settings,
        uint16_t gapwidth
        )
{
    settings->gapwidth = gapwidth;
}

uint16_t
USGetGapWidth(
        UserSettings *settings
        )
{
    return settings->gapwidth;
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

void
USSetBarHeight(
        UserSettings *settings,
        uint16_t height
        )
{
    settings->barheight = height;
}
uint16_t
USGetBarHeight(
        UserSettings *settings
        )
{   
    return settings->barheight;
}
void
USSetBarWidth(
        UserSettings *settings,
        uint16_t width 
        )
{  
    settings->barwidth = width;
}
uint16_t
USGetBarWidth(
        UserSettings *settings
        )
{   
    return settings->barwidth;
}
void
USSetBarX(
        UserSettings *settings,
        int16_t x
        )
{
    settings->barx = x;
}
uint16_t
USGetBarX(
        UserSettings *settings
        )
{   return settings->barx;
}
void
USSetBarY(
        UserSettings *settings,
        int16_t y
        )
{
    settings->bary = y;
}
uint16_t
USGetBarY(
        UserSettings *settings
        )
{
    return settings->bary;
}
