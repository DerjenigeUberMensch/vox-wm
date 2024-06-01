#ifndef USER_SETTINGS
#define USER_SETTINGS

#include <stdint.h>

/* User Settings Flags */

#define _USER_SETTINGS_HOVER_FOCUS          ((1 << 0))
#define _USER_SETTINGS_X                    ((1 << 1))
#define _USER_SETTINGS_A        ((1 << 2))
#define _USER_SETTINGS_B        ((1 << 3))
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


typedef struct UserSettings UserSettings;

struct UserSettings
{
    float mfact;
    float gapratio;

    uint16_t mcount;        /* tiling master count */
    uint8_t defaultlayout;
    uint8_t odefaultlayout;

    uint16_t defaultdesk;

    uint16_t snap;
    uint16_t refreshrate;

    uint16_t flags;
    uint16_t maxcc;

    /* bar stuff */
    uint16_t barheight;
    uint16_t barwidth;
    int16_t barx;
    int16_t bary;
};



void
USSetHoverFocus(
        UserSettings *settings, 
        uint8_t state
        );
uint8_t
USIsHoverFocus(
        UserSettings *settings
        );
void
USSetMFact(
        UserSettings *settings, 
        float mfact
        );
float
USGetMFact(
        UserSettings *settings
        );
void
USSetMCount(
        UserSettings *settings,
        uint16_t mcount
        );
uint16_t
USGetMCount(
        UserSettings *settings
        );
void
USSetLayout(
        UserSettings *settings,
        uint8_t layout
        );
uint8_t
USGetLayout(
        UserSettings *settings
        );
void
USSetOLayout(
        UserSettings *settings,
        uint8_t layout
        );
uint8_t
USGetOLayout(
        UserSettings *settings
        );
void
USSetDefaultDesk(
        UserSettings *settings,
        uint16_t desk 
        );
uint16_t
USGetDefaultDesk(
        UserSettings *settings
        );
void
USSetGapRatio(
        UserSettings *settings,
        float ratio
        );
float
USGetGapRatio(
        UserSettings *settings
        );
void
USSetSnap(
        UserSettings *settings,
        uint16_t snap
        );
uint16_t
USGetSnap(
        UserSettings *settings
        );
void
USSetRefreshRate(
        UserSettings *settings,
        uint16_t rate
        );
uint16_t
USGetRefreshRate(
        UserSettings *settings
        );
void
USSetMaxClientCount(
        UserSettings *settings,
        uint16_t maxcc 
        );
uint16_t
USGetMaxClientCount(
        UserSettings *settings
        );
void
USSetBarHeight(
        UserSettings *settings,
        uint16_t height
        );
uint16_t
USGetBarHeight(
        UserSettings *settings
        );
void
USSetBarWidth(
        UserSettings *settings,
        uint16_t width 
        );
uint16_t
USGetBarWidth(
        UserSettings *settings
        );
void
USSetBarX(
        UserSettings *settings,
        int16_t x
        );
uint16_t
USGetBarX(
        UserSettings *settings
        );
void
USSetBarY(
        UserSettings *settings,
        int16_t y
        );
uint16_t
USGetBarY(
        UserSettings *settings
        );

#endif
