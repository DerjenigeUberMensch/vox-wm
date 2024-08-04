#ifndef USER_SETTINGS
#define USER_SETTINGS

#include <stdint.h>

#include "parser.h"

/* User Settings Flags */

typedef struct BarSettings BarSettings;
typedef struct UserSettings UserSettings;

struct BarSettings
{
    /* Holds Ratios of size(s) relative to the monitor 
     * 0.0f -> 1.0f
     */
    float lx;    /* Ratio of Monitor x offset    */
    float ly;    /* Ratio of Monitor y offset    */
    float lw;    /* Ratio of Monitor w size      */
    float lh;    /* Ratio of Monitor h size      */

    /* Holds Ratios of size(s) relative to the monitor 
     * 0.0f -> 1.0f
     */
    float rx;    /* Ratio of Monitor x offset    */
    float ry;    /* Ratio of Monitor y offset    */
    float rw;    /* Ratio of Monitor w size      */
    float rh;    /* Ratio of Monitor h size      */

    /* Holds Ratios of size(s) relative to the monitor 
     * 0.0f -> 1.0f
     */
    float tx;    /* Ratio of Monitor x offset    */
    float ty;    /* Ratio of Monitor y offset    */
    float tw;    /* Ratio of Monitor w size      */
    float th;    /* Ratio of Monitor h size      */

    /* Holds Ratios of size(s) relative to the monitor 
     * 0.0f -> 1.0f
     */
    float bx;    /* Ratio of Monitor x offset    */
    float by;    /* Ratio of Monitor y offset    */
    float bw;    /* Ratio of Monitor w size      */
    float bh;    /* Ratio of Monitor h size      */
};


struct UserSettings
{
    float mfact;
    float gapratio;

    uint16_t mcount;        /* tiling master count */
    uint16_t snap;
    uint16_t refreshrate;

    uint16_t flags;
    uint16_t maxcc;

    /* bar stuff */
    BarSettings bar;
    SCParser *cfg;
};

enum USFlags
{
    USUseHoverFocus = 1 << 0,
    USUseServerDecorations = 1 << 1,
    USUseClientDecorations = 1 << 2,
    USPreferClientDecorations = 1 << 3,
};

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



/* Initialize Settings */
void
USInit(
        UserSettings *settings_init
        );
/*
 * RETURN: EXIT_SUCCESS on Success
 * RETURN: EXIT_FAILURE on Failure
 */
int
USInitFile(
        void
        );
/* Save current settings */
void
USSave(
        UserSettings *settings
        );
/* Load save data into settings */
void
USLoad(
        UserSettings *settings
        );
/* Free settings data */
void
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
