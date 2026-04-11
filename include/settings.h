#ifndef __SETTINGS__H__
#define __SETTINGS__H__

#include <stdint.h>
#include <stddef.h>

#include "SCParser/parser.h"
#include "FNotify/fnotify.h"

#include "util.h"

enum
UserSettingType
{
    MFact,
    GapRatio,
    MCount,
    Snap,
    RefreshRate,

    HoverFocus,
    UseDecorations,
    UseClientSideDecorations,
    PreferClientSideDecorations,

    /* legacy */
    UseLegacyFloatingSystem,

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


/* non-extension compliant 'switch' statment. */
#define VOX_ADD_MEMBER_SCTypeNoType(DEFAULT_SETTING)     { .data64 = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeBOOL(DEFAULT_SETTING)       { .data8  = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeUCHAR(DEFAULT_SETTING)      { .data8  = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeCHAR(DEFAULT_SETTING)       { .data8i = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeUSHORT(DEFAULT_SETTING)     { .data16 = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeSHORT(DEFAULT_SETTING)      { .data16i= { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeUINT(DEFAULT_SETTING)       { .data32 = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeINT(DEFAULT_SETTING)        { .data32i= { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeFLOAT(DEFAULT_SETTING)      { .dataf  = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeDOUBLE(DEFAULT_SETTING)     { .datad  = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeLONG(DEFAULT_SETTING)       { .data64i= { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeULONG(DEFAULT_SETTING)      { .data64 = { DEFAULT_SETTING } }
#define VOX_ADD_MEMBER_SCTypeSTRING(DEFAULT_SETTING)     { .datav  = { DEFAULT_SETTING } }

#define VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING) \
        VOX_ADD_MEMBER_##TYPE(DEFAULT_SETTING)

#define VOX_ADD_MEMBER(NAME, TYPE, DEFAULT_SETTING, UPDATE_FUNCTION)\
        [NAME] =                                                        \
        {                                                               \
            .name = #NAME,                                              \
            .name_len = sizeof(#NAME),                                  \
            .update_func = UPDATE_FUNCTION,                             \
            .type = TYPE,                                               \
            .default_data = VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING), \
            .data = VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING)         \
        },

#define VOX_ADD_MEMBER_SETTING(NAME, TYPE, DEFAULT_SETTING, UPDATE_FUNCTION) \
        VOX_ADD_MEMBER(NAME, TYPE, DEFAULT_SETTING, UPDATE_FUNCTION)

#define VOX_GENERATE_SETTING_LIST(LIST_NAME, LIST) SCSetting LIST_NAME[] = { LIST };

/* User Settings Flags */

typedef struct SCSetting SCSetting;
typedef struct UserSettings UserSettings;

struct 
SCSetting
{
    Generic default_data;
    Generic data;

    char *const name;
    size_t name_len;

    void (*update_func)(Generic prev, Generic cur);
    enum SCType type;
};


struct 
UserSettings
{
    SCParser *cfg;          /* Cfg holder                                                       */
    SCSetting *holder;

    pthread_mutex_t mutex;
    pthread_cond_t exitcond;
    uint8_t use_threads;
    FNotify fnotify;   /* NOTE: Access of this pointer is restricted to USInit */
};


/* Initialize Settings */
void NonNull
USInit(
        UserSettings *settings_init,
        SCSetting *items
        );
/* Save current settings */
void NonNull
USSave(
        UserSettings *settings
        );
/* Load save data into settings */
void NonNull
USLoad(
        UserSettings *settings
        );

Generic
USDefaultSetting(
        UserSettings *settings,
        unsigned int setting
        );

Generic
USGetSetting(
        UserSettings *settings,
        unsigned int setting
        );

void
USSetSetting(
        UserSettings *settings,
        unsigned int setting,
        Generic data
        );

/* Free settings data */
void NonNull
USWipe(
        UserSettings *settings
        );

#endif
