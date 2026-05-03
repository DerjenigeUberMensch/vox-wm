#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/keysym.h>
#include <X11/XF86keysym.h> 

#include "wmlua/input.h"
#include "main.h"
#include "keybinds.h"


enum
WMModMasks
{
    WM_ALT = XCBMod1Mask,
    WM_NUMLOCK = XCBMod2Mask,
    WM_SUPER = XCBMod4Mask,
    WM_CTRL = XCBControlMask,
    WM_SHIFT = XCBShiftMask,
    WM_CAPSLOCK = XCBLockMask,
};

enum
WMButton
{
    WM_LMB = XCBButton1,
    WM_MMB = XCBButton2,
    WM_RMB = XCBButton3,
    WM_SCROLL_UP = XCBButton4,
    WM_SCROLL_DOWN = XCBButton5,
    WM_SCROLL_LEFT = 6,
    WM_SCROLL_RIGHT = 7,
};

enum
WMKeysyms
{
    WM_CAPS_LOCK = XK_Caps_Lock,
    WM_NUM_LOCK = XK_Num_Lock,
    WM_SCROLL_LOCK = XK_Scroll_Lock,
    WM_ESCAPE = XK_Escape,
    WM_RETURN = XK_Return, /* ENTER */
    WM_SPACE = XK_space,
    WM_BACKSPACE = XK_BackSpace,
    WM_DELETE = XK_Delete,
    WM_HOME = XK_Home,
    WM_END = XK_End,
    WM_INSERT = XK_Insert,
    WM_PAUSE = XK_Pause,
    WM_PRINT = XK_Print,
    WM_BREAK = XK_Break,
    WM_HELP = XK_Help,
    WM_MENU = XK_Menu,
    WM_PAGE_UP = XK_Page_Up,
    WM_PAGE_DOWN = XK_Page_Down,

    WM_MINUS        = XK_minus,
    WM_EQUALS       = XK_equal,
    WM_BACKQUOTE    = XK_grave, /* `~ */

    WM_LEFT_BRACKET = XK_bracketleft,
    WM_RIGHT_BRACKET= XK_bracketright,

    WM_SEMICOLON    = XK_semicolon,
    WM_APOSTROPHE   = XK_apostrophe,
    WM_COMMA        = XK_comma,
    WM_PERIOD       = XK_period,
    WM_SLASH        = XK_slash,
    WM_BACKSLASH    = XK_backslash,

    WM_PLUS   = XK_plus,
    WM_UNDERSCORE = XK_underscore,
    WM_COLON  = XK_colon,
    WM_QUOTE  = XK_quotedbl,
    WM_LESS   = XK_less,
    WM_GREATER= XK_greater,
    WM_PIPE   = XK_bar,
    WM_TILDE  = XK_asciitilde,

    WM_UP = XK_Up,
    WM_DOWN = XK_Down,
    WM_RIGHT = XK_Right,
    WM_LEFT = XK_Left,

    WM_KP_0 = XK_KP_0,
    WM_KP_1 = XK_KP_1,
    WM_KP_2 = XK_KP_2,
    WM_KP_3 = XK_KP_3,
    WM_KP_4 = XK_KP_4,
    WM_KP_5 = XK_KP_5,
    WM_KP_6 = XK_KP_6,
    WM_KP_7 = XK_KP_7,
    WM_KP_8 = XK_KP_8,
    WM_KP_9 = XK_KP_9,

    WM_KP_ENTER = XK_KP_Enter,
    WM_KP_DIVIDE = XK_KP_Divide,
    WM_KP_MULTIPLY = XK_KP_Multiply,
    WM_KP_SUBTRACT = XK_KP_Subtract,
    WM_KP_ADD = XK_KP_Add,
    WM_KP_DECIMAL = XK_KP_Decimal,

    WM_KP_UP    = XK_KP_Up,
    WM_KP_DOWN  = XK_KP_Down,
    WM_KP_LEFT  = XK_KP_Left,
    WM_KP_RIGHT = XK_KP_Right,

    WM_KP_HOME      = XK_KP_Home,
    WM_KP_END       = XK_KP_End,
    WM_KP_PAGE_UP   = XK_KP_Page_Up,
    WM_KP_PAGE_DOWN = XK_KP_Page_Down,
    WM_KP_INSERT    = XK_KP_Insert,
    WM_KP_DELETE    = XK_KP_Delete,

    WM_A = XK_a,
    WM_B = XK_b,
    WM_C = XK_c,
    WM_D = XK_d,
    WM_E = XK_e,
    WM_F = XK_f,
    WM_G = XK_g,
    WM_H = XK_h,
    WM_I = XK_i,
    WM_J = XK_j,
    WM_K = XK_k,
    WM_L = XK_l,
    WM_M = XK_m,
    WM_N = XK_n,
    WM_O = XK_o,
    WM_P = XK_p,
    WM_Q = XK_q,
    WM_R = XK_r,
    WM_S = XK_s,
    WM_T = XK_t,
    WM_U = XK_u,
    WM_V = XK_v,
    WM_W = XK_w,
    WM_X = XK_x,
    WM_Y = XK_y,
    WM_Z = XK_z,

    WM_TAB = XK_Tab,
    WM_F1 = XK_F1,
    WM_F2 = XK_F2,
    WM_F3 = XK_F3,
    WM_F4 = XK_F4,
    WM_F5 = XK_F5,
    WM_F6 = XK_F6,
    WM_F7 = XK_F7,
    WM_F8 = XK_F8,
    WM_F9 = XK_F9,
    WM_F10 = XK_F10,
    WM_F11 = XK_F11,
    WM_F12 = XK_F12,

    WM_MON_BRIGHTNESS_UP = XF86XK_MonBrightnessUp,
    WM_MON_BRIGHTNESS_DOWN = XF86XK_MonBrightnessDown,
    WM_KBD_LIGHT_ON_OFF = XF86XK_KbdLightOnOff,
    WM_KBD_BRIGHTNESS_UP = XF86XK_KbdBrightnessUp,
    WM_KBD_BRIGHTNESS_DOWN = XF86XK_KbdBrightnessDown,

    WM_STANDBY = XF86XK_Standby,
    WM_AUDIO_VOL_DOWN = XF86XK_AudioLowerVolume,
    WM_AUDIO_MUTE = XF86XK_AudioMute,
    WM_AUDIO_VOL_UP = XF86XK_AudioRaiseVolume,
    WM_AUDIO_PLAY = XF86XK_AudioPlay,
    WM_AUDIO_STOP = XF86XK_AudioStop,
    WM_AUDIO_PREV = XF86XK_AudioPrev,
    WM_AUDIO_NEXT = XF86XK_AudioNext,
    WM_HOME_PAGE = XF86XK_HomePage,
    WM_MAIL = XF86XK_Mail,
    WM_START = XF86XK_Start,
    WM_SEARCH = XF86XK_Search,
    WM_AUDIO_RECORD = XF86XK_AudioRecord,

    WM_CALCULATOR   = XF86XK_Calculator,
    WM_MEMO         = XF86XK_Memo,
    WM_TODO_LIST    = XF86XK_ToDoList,
    WM_CALENDAR     = XF86XK_Calendar,
    WM_POWER_DOWN   = XF86XK_PowerDown,
    WM_CONTRAST_ADJ = XF86XK_ContrastAdjust,
    WM_ROCKER_UP    = XF86XK_RockerUp,
    WM_ROCKER_DOWN  = XF86XK_RockerDown,
    WM_ROCKER_ENTER = XF86XK_RockerEnter,

     WM_BACK            = XF86XK_Back,
    WM_FORWARD         = XF86XK_Forward,
    WM_STOP            = XF86XK_Stop,
    WM_REFRESH         = XF86XK_Refresh,
    WM_POWER_OFF       = XF86XK_PowerOff,
    WM_WAKE_UP         = XF86XK_WakeUp,
    WM_EJECT           = XF86XK_Eject,
    WM_SCREEN_SAVER    = XF86XK_ScreenSaver,
    WM_WWW             = XF86XK_WWW,
    WM_SLEEP           = XF86XK_Sleep,
    WM_FAVORITES       = XF86XK_Favorites,
    WM_AUDIO_PAUSE     = XF86XK_AudioPause,
    WM_AUDIO_MEDIA     = XF86XK_AudioMedia,
    WM_MY_COMPUTER     = XF86XK_MyComputer,
    WM_VENDOR_HOME     = XF86XK_VendorHome,
    WM_LIGHT_BULB      = XF86XK_LightBulb,
    WM_SHOP            = XF86XK_Shop,
    WM_HISTORY         = XF86XK_History,
    WM_OPEN_URL        = XF86XK_OpenURL,
    WM_ADD_FAVORITE    = XF86XK_AddFavorite,
    WM_HOTLINKS        = XF86XK_HotLinks,
    WM_BRIGHTNESS_ADJ  = XF86XK_BrightnessAdjust,
    WM_FINANCE         = XF86XK_Finance,
    WM_COMMUNITY       = XF86XK_Community,
    WM_AUDIO_REWIND    = XF86XK_AudioRewind,
    WM_BACK_FORWARD    = XF86XK_BackForward,

    WM_LAUNCH_0 = XF86XK_Launch0,
    WM_LAUNCH_1 = XF86XK_Launch1,
    WM_LAUNCH_2 = XF86XK_Launch2,
    WM_LAUNCH_3 = XF86XK_Launch3,
    WM_LAUNCH_4 = XF86XK_Launch4,
    WM_LAUNCH_5 = XF86XK_Launch5,
    WM_LAUNCH_6 = XF86XK_Launch6,
    WM_LAUNCH_7 = XF86XK_Launch7,
    WM_LAUNCH_8 = XF86XK_Launch8,
    WM_LAUNCH_9 = XF86XK_Launch9,
    WM_LAUNCH_A = XF86XK_LaunchA,
    WM_LAUNCH_B = XF86XK_LaunchB,
    WM_LAUNCH_C = XF86XK_LaunchC,
    WM_LAUNCH_D = XF86XK_LaunchD,
    WM_LAUNCH_E = XF86XK_LaunchE,
    WM_LAUNCH_F = XF86XK_LaunchF,

    WM_APPLICATION_LEFT   = XF86XK_ApplicationLeft,
    WM_APPLICATION_RIGHT  = XF86XK_ApplicationRight,
    WM_BOOK               = XF86XK_Book,
    WM_CD                 = XF86XK_CD,
    WM_CALCULATOR_2       = XF86XK_Calculater,
    WM_CLEAR              = XF86XK_Clear,
    WM_CLOSE              = XF86XK_Close,
    WM_COPY               = XF86XK_Copy,
    WM_CUT                = XF86XK_Cut,
    WM_DISPLAY            = XF86XK_Display,
    WM_DOS                = XF86XK_DOS,
    WM_DOCUMENTS         = XF86XK_Documents,
    WM_EXCEL              = XF86XK_Excel,
    WM_EXPLORER           = XF86XK_Explorer,
    WM_GAME               = XF86XK_Game,
    WM_GO                 = XF86XK_Go,
    WM_ITOUCH             = XF86XK_iTouch,
    WM_LOG_OFF            = XF86XK_LogOff,
    WM_MARKET             = XF86XK_Market,
    WM_MEETING            = XF86XK_Meeting,
    WM_MENU_KB            = XF86XK_MenuKB,
    WM_MENU_PB            = XF86XK_MenuPB,
    WM_MY_SITES           = XF86XK_MySites,
    WM_NEW                = XF86XK_New,
    WM_NEWS               = XF86XK_News,
    WM_OFFICE_HOME        = XF86XK_OfficeHome,
    WM_OPEN               = XF86XK_Open,
    WM_OPTION             = XF86XK_Option,
    WM_PASTE              = XF86XK_Paste,
    WM_PHONE              = XF86XK_Phone,
    WM_MEDIA_Q            = XF86XK_Q,           /* Compaq's Q - don't use      */
    WM_REPLY              = XF86XK_Reply,
    WM_RELOAD             = XF86XK_Reload,
    WM_ROTATE_WINDOWS     = XF86XK_RotateWindows,
    WM_ROTATION_PB        = XF86XK_RotationPB,
    WM_ROTATION_KB        = XF86XK_RotationKB,
    WM_SAVE               = XF86XK_Save,
    WM_MEDIA_SCROLL_UP    = XF86XK_ScrollUp,        /* Scroll window/contents up   */
    WM_MEDIA_SCROLL_DOWN  = XF86XK_ScrollDown,      /* Scroll window/contents down */
    WM_SCROLL_CLICK       = XF86XK_ScrollClick,
    WM_SEND               = XF86XK_Send,
    WM_SPELL              = XF86XK_Spell,
    WM_SPLIT_SCREEN       = XF86XK_SplitScreen,
    WM_SUPPORT            = XF86XK_Support,
    WM_TASK_PANE          = XF86XK_TaskPane,
    WM_TERMINAL           = XF86XK_Terminal,
    WM_TOOLS              = XF86XK_Tools,
    WM_TRAVEL             = XF86XK_Travel,       /* ?? */
    WM_USER_PB            = XF86XK_UserPB,       /* ?? */
    WM_USER1_KB           = XF86XK_User1KB,      /* ?? */
    WM_USER2_KB           = XF86XK_User2KB,      /* ?? */
    WM_VIDEO              = XF86XK_Video,
    WM_WHEEL_BUTTON       = XF86XK_WheelButton,
    WM_WORD               = XF86XK_Word,
    WM_XFER               = XF86XK_Xfer,
    WM_ZOOM_IN            = XF86XK_ZoomIn,
    WM_ZOOM_OUT           = XF86XK_ZoomOut,

    WM_AWAY                = XF86XK_Away,
    WM_MESSENGER           = XF86XK_Messenger,
    WM_WEBCAM              = XF86XK_WebCam,
    WM_MAIL_FORWARD        = XF86XK_MailForward,
    WM_PICTURES            = XF86XK_Pictures,
    WM_MUSIC               = XF86XK_Music,

    WM_BATTERY             = XF86XK_Battery,
    WM_BLUETOOTH           = XF86XK_Bluetooth,
    WM_WLAN                = XF86XK_WLAN,
    WM_UWB                 = XF86XK_UWB,

    WM_AUDIO_FORWARD       = XF86XK_AudioForward,
    WM_AUDIO_REPEAT        = XF86XK_AudioRepeat,
    WM_AUDIO_RANDOM_PLAY   = XF86XK_AudioRandomPlay,
    WM_SUBTITLE            = XF86XK_Subtitle,
    WM_AUDIO_CYCLE_TRACK   = XF86XK_AudioCycleTrack,
    WM_CYCLE_ANGLE         = XF86XK_CycleAngle,
    WM_FRAME_BACK          = XF86XK_FrameBack,
    WM_FRAME_FORWARD       = XF86XK_FrameForward,
    WM_TIME                = XF86XK_Time,
    WM_SELECT              = XF86XK_Select,
    WM_VIEW                = XF86XK_View,
    WM_TOP_MENU            = XF86XK_TopMenu,

    WM_RED                 = XF86XK_Red,
    WM_GREEN               = XF86XK_Green,
    WM_YELLOW              = XF86XK_Yellow,
    WM_BLUE                = XF86XK_Blue,

    WM_SUSPEND             = XF86XK_Suspend,
    WM_HIBERNATE           = XF86XK_Hibernate,
    WM_TOUCHPAD_TOGGLE     = XF86XK_TouchpadToggle,
    WM_TOUCHPAD_ON         = XF86XK_TouchpadOn,
    WM_TOUCHPAD_OFF        = XF86XK_TouchpadOff,

    WM_AUDIO_MIC_MUTE      = XF86XK_AudioMicMute,
    WM_KEYBOARD            = XF86XK_Keyboard,

    WM_WWAN                = XF86XK_WWAN,
    WM_RF_KILL             = XF86XK_RFKill,

    WM_AUDIO_PRESET        = XF86XK_AudioPreset,

    WM_SWITCH_VT_1         = XF86XK_Switch_VT_1,
    WM_SWITCH_VT_2         = XF86XK_Switch_VT_2,
    WM_SWITCH_VT_3         = XF86XK_Switch_VT_3,
    WM_SWITCH_VT_4         = XF86XK_Switch_VT_4,
    WM_SWITCH_VT_5         = XF86XK_Switch_VT_5,
    WM_SWITCH_VT_6         = XF86XK_Switch_VT_6,
    WM_SWITCH_VT_7         = XF86XK_Switch_VT_7,
    WM_SWITCH_VT_8         = XF86XK_Switch_VT_8,
    WM_SWITCH_VT_9         = XF86XK_Switch_VT_9,
    WM_SWITCH_VT_10        = XF86XK_Switch_VT_10,
    WM_SWITCH_VT_11        = XF86XK_Switch_VT_11,
    WM_SWITCH_VT_12        = XF86XK_Switch_VT_12,

    WM_UNGRAB              = XF86XK_Ungrab,
    WM_CLEAR_GRAB          = XF86XK_ClearGrab,
    WM_NEXT_VMODE          = XF86XK_Next_VMode,
    WM_PREV_VMODE          = XF86XK_Prev_VMode,
    WM_LOG_WINDOW_TREE     = XF86XK_LogWindowTree,
    WM_LOG_GRAB_INFO       = XF86XK_LogGrabInfo,
};

typedef struct KeyCodeEntry KeyCodeEntry;

struct
KeyCodeEntry
{
    const char *name;
    u32 keycode;
};

static const KeyCodeEntry mods_table[] =
{
    { "alt", WM_ALT },
    { "numlock", WM_NUMLOCK },
    { "super", SUPER },
    { "windowkey", SUPER },
    { "command", SUPER },

    { "capslock", WM_CAPSLOCK },
    { "ctrl", WM_CTRL },
    { "shift", WM_SHIFT },
};

static const KeyCodeEntry keycode_table[] =
{ 
    { "tab", TAB },
    { "escape", ESCAPE },

    { "lmb", LMB },

    { "mmb", MMB },

    { "rmb", RMB },

    { "scrollup", ScrollUp },
    { "scrolldown", ScrollDown },
    { "scrollleft", ScrollLeft },
    { "scrollright", ScrollRight},

    { "capslock", CAPSLOCK },
    { "caplock", CAPSLOCK },

    { "enter", ENTER },
    { "return", ENTER },

    { "f1", XF1 },
    { "f2", XF2 },
    { "f3", XF3 },
    { "f4", XF4 },
    { "f5", XF5 },
    { "f6", XF6 },
    { "f7", XF7 },
    { "f8", XF8 },
    { "f9", XF9 },
    { "f10", XF10 },
    { "f11", XF11 },
    { "f12", XF12 },

    { "", },

};


extern WM _wm;

int 
strcmp_lower(const char *a, const char *b)
{
    char ca;
    char cb;

    while (*a && *b) 
    {
        ca = tolower((unsigned char)*a);
        cb = tolower((unsigned char)*b);

        if (ca != cb) 
        {   return ca - cb;
        }

        ++a;
        ++b;
    }

    return *a - *b;
}

int 
l_input_bind(lua_State *l)
{
    const char *combo = lua_tostring(l, 1);

    if(!combo || combo[0] == '\0')
    {   return luaL_error(l, "invalid bind");
    }

    enum { BUFF_SIZE = 1024 };

    char buff[BUFF_SIZE];

    memset(buff, 0, BUFF_SIZE);

    strncpy(buff, combo, (BUFF_SIZE - 1) * sizeof(char));

    u32 mask = 0;
    u32 i = 0;
    XCBKeysym keycode = 0;

    char *saveptr;
    char *token = strtok_r(buff, "+", &saveptr);

    while (token)
    {
        bool found = false;

        // modifier check
        for (i = 0; i < LENGTH(mods_table); ++i)
        {
            if (!strcmp_lower(token, mods_table[i].name))
            {
                mask |= mods_table[i].keycode;
                found = true;
                break;
            }
        }

        // keycode check
        if (!found)
        {
            for (i = 0; i < LENGTH(keycode_table); ++i)
            {
                if (!strcmp_lower(token, keycode_table[i].name))
                {
                    keycode = keycode_table[i].keycode;
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            DebugWarn("no found");
            break;
        }

        token = strtok_r(NULL, "+", &saveptr);
    }
}
