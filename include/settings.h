#ifndef __SETTINGS__H__
#define __SETTINGS__H__

#include <stdint.h>

#include "SCParser/parser.h"
#include "FNotify/fnotify.h"

#include "util.h"

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
#define VOX_ADD_MEMBER_SCTypeSTRING(DEFAULT_SETTING)     { .v      = { DEFAULT_SETTING } }

#define VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING) \
        VOX_ADD_MEMBER_##TYPE(DEFAULT_SETTING)

#define VOX_ADD_MEMBER(NAME, TYPE, OFFSET, SIZE, DEFAULT_SETTING, UPDATE_FUNCTION)\
        [NAME] =                                                        \
        {                                                               \
            .name = #NAME,                                              \
            .name_len = sizeof(#NAME),                                  \
            .update_func = UPDATE_FUNCTION,                             \
            .type = TYPE,                                               \
            .offset = OFFSET,                                           \
            .size = SIZE,                                               \
            .default_data = VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING), \
            .data = VOX_ADD_MEMBER_TYPED(TYPE, DEFAULT_SETTING)         \
        },

#define VOX_ADD_MEMBER_SETTING(NAME, TYPE, DEFAULT_SETTING, UPDATE_FUNCTION) \
        VOX_ADD_MEMBER(NAME, TYPE, offsetof(UserSettings, NAME), FIELD_SIZEOF(UserSettings, NAME), DEFAULT_SETTING, UPDATE_FUNCTION)

#define VOX_GENERATE_SETTING_LIST(LIST_NAME, LIST) SCSetting LIST_NAME[] = { LIST };

/* User Settings Flags */

typedef struct SCSetting SCSetting;
typedef struct UserSettings UserSettings;

struct 
SCSetting
{
    const Generic default_data;
    const char *const name;
    const void (*update_func)(Generic prev, Generic cur);
    const enum SCType type;
    const uint8_t name_len;
    const uint8_t size;
    const uint16_t offset;
    Generic data;
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


/* Setter getters */

uint32_t HASHOVERFOCUS(UserSettings *settings);
uint32_t HASSERVERDECOR(UserSettings *settings);
uint32_t HASCLIENTDECOR(UserSettings *settings);
uint32_t PREFERCLIENTDECOR(UserSettings *settings);

void USSetHoverFocus(UserSettings *settings, uint8_t state);
void USSetUseServerDecor(UserSettings *settings, uint8_t state);
void USSetUseClientDecor(UserSettings *settings, uint8_t state);
void USSetPreferClientDecor(UserSettings *settings, uint8_t state);

#endif
