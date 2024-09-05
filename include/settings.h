#ifndef USER_SETTINGS
#define USER_SETTINGS

#include <stdint.h>

#include "parser.h"

/* User Settings Flags */

typedef struct UserSettings UserSettings;

struct 
UserSettings
{
    SCParser *cfg;          /* Cfg holder                                                       */

    float mfact;            /* factor of master area size [0.05..0.95]                          */
    float gapratio;         /* invisible border pixel of windows (CFG_BORDER_PX not affected)   */
    uint16_t mcount;        /* number of clients in master area                                 */
    uint16_t snap;          /* snap window to border in pixels; 0 to disable (NOT RECOMMENDED)  */
    uint16_t refreshrate;   /* max refresh rate when resizing, moving windows;  0 to disable    */

    uint16_t flags;
    uint16_t maxcc;         /* max number of clients (XOrg Default is 256)                      */


    /* Bar Setting Data */
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

enum 
USFlags
{
    USUseHoverFocus = 1 << 0,
    USUseServerDecorations = 1 << 1,
    USUseClientDecorations = 1 << 2,
    USPreferClientDecorations = 1 << 3,
};

enum
{
    MFact,
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
