#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <stdio.h>

#include "uthash.h"
#include "bar.h"
#include "settings.h"
#include "pannel.h"
#include "XCB-TRL/xcb_trl.h"
#include "XCB-TRL/xcb_winutil.h"


#ifndef VERSION
#define VERSION     "NOT_SET.NOT_SET.NOT_SET"
#endif

#ifndef NAME
#define NAME        "NOT_SET"
#endif

/* CONSTANTS */
#define BUTTONMASK              (XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE)
#define CLEANMASK(mask)         (mask & ~(_wm.numlockmask|XCB_MOD_MASK_LOCK) & \
                                (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL| \
                                 XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|XCB_MOD_MASK_4|XCB_MOD_MASK_5))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                                * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK | XCB_EVENT_MASK_POINTER_MOTION)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TAGSLENGTH              (LENGTH(tags))
#define SESSION_FILE            "/tmp/dwm-session"
#define CONFIG_FILE             "/var/tmp/dwm-config"   /* todo make dir .config/dwm/config or someting like that */
#define BORKED                  "NOT_SET"
#define MANAGE_CLIENT_COOKIE_COUNT 16 + 1

/* Client struct flags */
#define _FSTATE_FLOATING        ((1 << 0))
#define _FSTATE_WASFLOATING     ((1 << 1))
#define _FSTATE_SHOW_DECOR      ((1 << 2))

/* EWMH window types */
#define _TYPE_DESKTOP       ((1 << 0))
#define _TYPE_DOCK          ((1 << 1))
#define _TYPE_TOOLBAR       ((1 << 2))
#define _TYPE_MENU          ((1 << 3))
#define _TYPE_UTILITY       ((1 << 4))
#define _TYPE_SPLASH        ((1 << 5))
#define _TYPE_DIALOG        ((1 << 6))
#define _TYPE_DROPDOWN_MENU ((1 << 7))
#define _TYPE_POPUP_MENU    ((1 << 8))
#define _TYPE_TOOLTIP       ((1 << 9))
#define _TYPE_NOTIFICATION  ((1 << 10))
#define _TYPE_COMBO         ((1 << 11))
#define _TYPE_DND           ((1 << 12))
#define _TYPE_NORMAL        ((1 << 13))

/* custom types (using spare bits )*/
#define _TYPE_NEVERFOCUS    ((1 << 14))
/* Window map states, Widthdrawn, Iconic, Normal. */
#define _TYPE_MAP_ICONIC        ((1 << 15))

/* EWMH Window states */
#define _STATE_MODAL                        ((1 << 0))
#define _STATE_STICKY                       ((1 << 1))
#define _STATE_MAXIMIZED_VERT               ((1 << 2))  
#define _STATE_MAXIMIZED_HORZ               ((1 << 3))
#define _STATE_SHADED                       ((1 << 4))
#define _STATE_SKIP_TASKBAR                 ((1 << 5))
#define _STATE_SKIP_PAGER                   ((1 << 6))
#define _STATE_HIDDEN                       ((1 << 7))
#define _STATE_FULLSCREEN                   ((1 << 8))
#define _STATE_ABOVE                        ((1 << 9))
#define _STATE_BELOW                        ((1 << 10))
#define _STATE_DEMANDS_ATTENTION            ((1 << 11))
#define _STATE_FOCUSED                      ((1 << 12))

/* extra states (using spare bits) */
#define _STATE_SUPPORTED_WM_TAKE_FOCUS      ((1 << 13))
#define _STATE_SUPPORTED_WM_SAVE_YOURSELF   ((1 << 14))
#define _STATE_SUPPORTED_WM_DELETE_WINDOW   ((1 << 15))

/* cursor */
enum CurType 
{ 
    CurNormal, 
    CurResizeTopL, 
    CurResizeTopR, 
    CurMove, 
    CurLast 
}; 
/* color schemes */
enum SchemeType 
{ 
    SchemeNorm, 
    SchemeSel 
};
/* clicks */
enum ClkType 
{ 
    ClkTagBar, 
    ClkLtSymbol, 
    ClkStatusText, 
    ClkWinTitle,
    ClkClientWin, 
    ClkRootWin, 
    ClkLast 
};
/* kill client type */
enum KillType 
{ 
    Graceful, 
    Safedestroy, 
    Destroy, 
};

/* layout(s) */
enum LayoutType
{
    Tiled, Floating, Monocle, Grid, LayoutTypeLAST
};

enum ClientListModes
{
    ClientListAdd, ClientListRemove, ClientListReload,
};

enum BarSides
{
    BarSideLeft, BarSideRight, BarSideTop, BarSideBottom
};

typedef union  Arg Arg;
typedef struct Key Key;
typedef struct Button Button;
typedef struct Monitor Monitor;
typedef struct Client Client;
typedef struct Decoration Decoration;
typedef struct Stack Stack;
typedef struct Layout Layout;
typedef struct Desktop Desktop;
typedef struct WM WM;

union Arg
{
    int32_t i;              /* i  -> int            */
    uint32_t ui;            /* ui -> unsigned int   */
    int64_t l;              /* l  -> long int       */
    uint64_t ul;            /* ul -> unsigned long  */
    float f;                /* f  -> float          */
    double d;               /* d  -> double         */
    void *v;                /* v  -> void pointer   */
};

struct Key
{
    uint16_t type;              /* KeyPress/KeyRelease  */
    uint16_t mod;               /* Modifier             */
    XCBKeysym keysym;           /* Key symbol           */
    void (*func)(const Arg *);  /* Function             */
    Arg arg;                    /* Argument             */
    UT_hash_handle hh;          /* Hash                 */
};

struct Button
{
    uint8_t type;                   /* ButtonPress/ButtonRelease    */
    uint8_t button;                 /* Button                       */
    uint16_t mask;                  /* Modifier                     */
    void (*func)(const Arg *arg);   /* Function                     */
    Arg arg;                        /* Argument                     */
};

struct Client
{
    int16_t x;          /* X coordinate             */
    int16_t y;          /* Y coordinate             */

    uint16_t w;         /* Width                    */
    uint16_t h;         /* height                   */


    int16_t oldx;       /* Previous X coordinate    */
    int16_t oldy;       /* Previous Y coordinate    */

    uint16_t oldw;      /* Previous Width           */
    uint16_t oldh;      /* Previous Height          */


    uint16_t wtypeflags;/* Window type flags        */
    uint16_t wstateflags;/* Window state flags      */

    uint32_t flags;     /* Misc States              */


    uint16_t bw;        /* Border Width             */
    uint16_t oldbw;     /* Old Border Width         */

    uint32_t bcol;      /* Border Colour            */


    float mina;         /* Minimum Aspect           */
    float maxa;         /* Maximum Aspect           */


    uint16_t basew;     /* Base Width               */
    uint16_t baseh;     /* Base Height              */

    uint16_t incw;      /* Increment Width          */
    uint16_t inch;      /* Increment Height         */


    uint16_t maxw;      /* Max Width.               */
    uint16_t maxh;      /* Max Height.              */

    uint16_t minw;      /* Minimum Width            */
    uint16_t minh;      /* Minimum Height           */


    XCBWindow win;      /* Client Window            */
    pid_t pid;          /* Client Pid               */


    Client *next;       /* The next client in list  */
    Client *prev;       /* The previous client      */
    Client *sprev;      /* The prev stack order clnt*/
    Client *snext;      /* The next client in stack */
    Client *rnext;      /* Restack Next             */
    Client *rprev;      /* Restack Prev             */
    Client *fnext;      /* The next focused client  */
    Client *fprev;      /* THe previous focused clnt*/
    Desktop *desktop;   /* Client Associated Desktop*/
    Decoration *decor;  /* Decoration AKA title bar.*/

    char *netwmname;    /* Client Name              */
    char *wmname;       /* Client Name backup       */
    char *classname;    /* Class Name               */
    char *instancename; /* Instance Name            */
    uint32_t *icon;     /* Array of icon values     */

    UT_hash_handle hh;  /* hash handle              */
    uint16_t rstacknum; /* Used in calculating pos  */
    uint8_t pad0[6];
};

struct Decoration
{
    /* TODO */
    uint16_t w;
    uint16_t h;
    XCBWindow win;
};

struct Monitor
{
    int16_t mx;                 /* Monitor X (Screen Area)                  */
    int16_t my;                 /* Monitor Y (Screen Area)                  */
    uint16_t mw;                /* Monitor Width (Screen Area)              */
    uint16_t mh;                /* Monitor Height (Screen Area)             */
    int16_t wx;                 /* Monitor X (Window Area)                  */
    int16_t wy;                 /* Monitor Y (Window Area)                  */
    uint16_t ww;                /* Monitor Width (Window Area)              */
    uint16_t wh;                /* Monitor Height (Window Area)             */

    Desktop *desktops;          /* First Desktop in linked list             */
    Desktop *desklast;          /* Last Desktop                             */
    Desktop *desksel;           /* Selected Desktop                         */
    Monitor *next;              /* Next Monitor                             */
    Bar *bar;                   /* The Associated Task-Bar                  */
    Client *__hash;             /* Hashed clients                           */

    uint16_t deskcount;         /* Desktop Counter                          */
    uint8_t pad0[6];
};

struct Layout
{   
    void (*arrange)(Desktop *);
};

struct Desktop
{
    int16_t num;                /* The Desktop Number           */
    
    uint8_t layout;             /* The Layout Index             */
    uint8_t olayout;            /* The Previous Layout Index    */

    Monitor *mon;               /* Desktop Monitor              */
    Client *clients;            /* First Client in linked list  */
    Client *clast;              /* Last Client in linked list   */
    Client *stack;              /* Client Stack Order           */
    Client *slast;              /* Last Client in Stack         */
    Client *rstack;             /* restack Client order         */
    Client *rlast;              /* Last restack Client          */
    Client *focus;              /* Client Focus Order           */
    Client *flast;              /* Client Last Focus            */
    Client *sel;                /* Selected Client              */
    Desktop *next;              /* Next Client in linked list   */
    Desktop *prev;              /* Previous Client in list      */
    UserSettings *settings;     /* User settings data           */
};

struct WM
{
    int screen;                     /* Screen id            */
    int numlockmask;                /* numlockmask          */

    int running;                    /* Running flag         */

    uint8_t restart;                /* Restart flag         */
    uint8_t has_error;              /* Error flag           */
    uint8_t pad[2];                 /* Pad                  */

    uint16_t sw;                    /* Screen Height u16    */
    uint16_t sh;                    /* Screen Width  u16    */

    XCBWindow root;                 /* The root window      */
    XCBWindow wmcheckwin;           /* window manager check */

    XCBDisplay *dpy;                /* The current display  */
    Monitor *selmon;                /* Selected Monitor     */
    Monitor *mons;                  /* Monitors             */
    XCBKeySymbols *syms;            /* keysym alloc         */
    char *wmname;                   /* WM_NAME              */
};


/* Handles the main(int argc, char **argv) arguments. */
void argcvhandler(int argc, char *argv[]);
/* Applies the gravity shifts specified by the gravity onto the x and y coordinates.
*/
void applygravity(const uint32_t gravity, int16_t *x, int16_t *y, const uint16_t width, const uint16_t height, const uint16_t border_width);
/* Applies size hints to the specified values.
* interact:             1/true/True         Does not restrict bounds to window area.
*                       0/false/False       Restricts bounds to window area.
* RETURN: 1 if the specified x/y/w/h does not match after sizehints applied. (aka need to resize.)
* RETURN: 0 if the specified x/y/w/h does match after the sizehints applied. (No need to resize.)
*/
uint8_t applysizehints(Client *c, int32_t *x, int32_t *y, int32_t *width, int32_t *height, uint8_t interact);
/* quickly calculate arrange stuff */
void arrangeq(Desktop *desk);
/* Arranges and restacks the windows in the specified desktop.
*/
void arrange(Desktop *desk);
/* Arranges and restacks all the windows for every deskop in the specified monitor.
*/
void arrangemon(Monitor *m);
/* Arrange and restacks every window on all monitors.
*/
void arrangemons(void);
/* Arranges the windows in the specified desktop.
 *
 * NOTE: Does not restack windows.
 */
void arrangedesktop(Desktop *desk);
/* Adds desktop to specified monitor linked list.
*/
void attachdesktop(Monitor *m, Desktop *desk);
/* Removes desktop fromt specified monitor linked list.
*/
void detachdesktop(Monitor *m, Desktop *desk);
/* Adds Client to clients desktop linked list.
*/
void attach(Client *c);
/* Adds Client to rendering stack order in desktop linked list.
*/
void attachstack(Client *c);
/* Adds Client to previous rendering stack order.
 */
void attachrestack(Client *c);
/* Adds Client to focus linked list. 
 */
void attachfocus(Client *c);
void attachfocusafter(Client *start, Client *after);
void attachfocusbefore(Client *start, Client *after);
/* Removes Client from clients desktop linked list.
*/
void detach(Client *c);
/* Removes all connections from clients desktop linked list
 * Analagous to detachstack(c) and detach(c);
*/
void detachcompletely(Client *c);
/* Removes Client from desktop rendering stack order.
*/
void detachstack(Client *c);
/* Removes Client from previous restack order. (rstack);
 */
void detachrestack(Client *c);
/* Removes Client from desktop focus order.
*/
void detachfocus(Client *c);
/* Checks given the provided information if a window is eligible to be a new bar.
 * RETURN: 1 on True.
 * RETURN: 0 on False
*/
uint8_t checknewbar(Client *c, const uint8_t has_strut_or_strut_partial);
/* Inital startup check if there is another window manager running.
*/
void checkotherwm(void);
/* Checks if a given number would be sticky in the wm-spec.
 * RETURN: NonZero on True.
 * RETURN: 0 on False.
 */
uint8_t checksticky(int64_t x);
/* Cleanups and frees any data previously allocated.
*/
void cleanup(void);
/* Frees Client and allocated client properties. 
*/
void cleanupclient(Client *c);
/* Frees allocated cursors.
 */
void cleanupcursors(void);
/* Frees desktop and allocated desktop properties.
*/
void cleanupdesktop(Desktop *desk);
/* Frees Monitor and allocated Monitor properties.
*/
void cleanupmon(Monitor *m);
/* Frees all monitors and allocated Monitor properties.
*/
void cleanupmons(void);
/* Initializes the Client geometry from the specified XCBWindowGeometry struct. 
 */
void clientinitgeom(Client *c, XCBWindowGeometry *geometry);
/* Initializes the Client window type from the specified XCBWindowProperty. */
void clientinitwtype(Client *c, XCBWindowProperty *windowtypereply);
/* Initializes the Client window state from the specified XCBWindowProperty. */
void clientinitwstate(Client *c, XCBWindowProperty *windowstatereply);
/* Sets the correct client desktop if trans found, default to _wm.selmon->desksel if not.*/
void clientinittrans(Client *c, XCBWindow trans);
/* Updates the XServers knowledge of the clients coordinates.
 * NOTE: This is a sendevent to the c->win data type.
 * NOTE: XCBFlush(); must be called to push the XCB internal buffer to send this request.
 */
void configure(Client *c);
/* Allocates a client and client properties with all data set to 0 or the adress of any newly allocated data.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
*/
Client *createclient(void);
/* Allocates a bar and bar properties with all data set to 0 or to the adress of any newly allocated data.
 * RETURN: Bar * on Success.
 * RETURN: NULL on Failure.
*/
Bar *createbar(void);
/* Allocates a desktop and desktop properties with all data set to 0 or to the adress of any newly allocated data.
 * RETURN: Desktop * on Success.
 * RETURN: NULL on Failure.
 */
Desktop *createdeskop(void);
/* Allocates a Monitor and Monitor properties with all data set to 0 or to the adress of any newly allocated data.
 * RETURN: Monitor * on Success.
 * RETURN: exit(1) on Failure.
 */
Monitor *createmon(void);
/* Allocates a decoration with all properties set to 0 or NULL. 
 * RETURN: Decoration * on Success.
 * RETURN: NULL on Failure.
 */
Decoration *createdecoration(void);
/* Finds the next monitor based on "dir" AKA Direction. 
 * RETURN: Monitor * on Success.
 * RETURN: NULL on Failure.
 */
Monitor *dirtomon(uint8_t dir);
/* Deprecated !ISFLOATING(c) now has this behaviour. */
uint8_t docked(Client *c);
/* Jumps to the specified function handler for the provided event.
*/
void eventhandler(XCBGenericEvent *ev);
/* handles atexit.
*/
void exithandler(void);
/* Sets the "floating" layout for the specified desktop.
 * Floating -> Windows overlap each other AKA default win10 window behaviour.
 */
void floating(Desktop *desk);
/* Sets focus to the specified client.
 * NOTE: if NULL provided first visible client in stack is choosen as focus specifier.
 */
void focus(Client *c);
/* UNUSED/TODO
 */
int32_t getstate(XCBWindow win, XCBGetWindowAttributes *state);
/* Allocates memory and resturns the pointer in **str_return from the specified XCBWindowProperty. */
void getnamefromreply(XCBWindowProperty *namerep, char **str_return);
/* Gets the icon property from the specified XCBWindowProperty. */
uint32_t *geticonprop(XCBWindowProperty *iconreply);
/* Grabs a windows buttons. 
 * Basically this just allows us to receive button press/release events from windows.
 */
void grabbuttons(XCBWindow window, uint8_t focused);
/* Grabs a windows keys.
 * Basically this just allows us to receive/intercept key press/release events.
 *
 */
void grabkeys(void);
/* Sets the "grid" layout for the specified desktop.
 * grid -> windows are sorted in a grid like formation, like those ones in hacker movies.
 */
void grid(Desktop *desk);
/* Kills the specified window.
 * type:            Graceful            Sends a message to the window to kill itself.
 *                  Safedestroy         Sends a message to the window to kill itself, on failure, forcefully kill the window.
 *                  Destroy             Destroys a window without sending any message for the window to response (Nuclear option.)
 */
void killclient(Client *c, enum KillType type);
/* requests for clients cookies. */
void managerequest(XCBWindow win, XCBCookie requests[MANAGE_CLIENT_COOKIE_COUNT]);
/* Part of main event loop "run()"
 * Manages AKA adds the window to our current or windows specified desktop.
 * Applies size checks, bounds, layout, etc...
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *managereply(XCBWindow window, XCBCookie requests[MANAGE_CLIENT_COOKIE_COUNT]);
/* Sets the window to the specified bar, if the bar doesnt exist it creates it.
 * RETURN: Bar * on Success.
 * RETURN: NULL on Failure.
 */
Bar *managebar(Monitor *m, XCBWindow win);
/* Maximizes a client if unmaxed, Sets flag.
 */
void maximize(Client *c);
/* Maximizes horizontally a client if unmaxed horz, Sets flag.
 */
void maximizehorz(Client *c);
/* Maximizes vertically a client if unmaxed vert, Sets flag.
 */
void maximizevert(Client *c);
/* Sets the "monocle" layout for the specified desktop.
 * monocle -> Windows are maximized to the screen avaible area, 
 * while floating windows are always raised above all others.
 */
void monocle(Desktop *desk);
/* Returns the next client avaible.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *nextclient(Client *c);
/* Returns the next Desktop avaible.
 * RETURN: Desktop* on Success.
 * RETURN: NULL on Failure.
 */
Desktop *nextdesktop(Desktop *desktop);
/* Returns the next Monitor avaible.
 * RETURN: Monitor* on Success.
 * RETURN: NULL on Failure.
 */
Monitor *nextmonitor(Monitor *monitor);
/* Returns the next client in stack avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nextstack(Client *c);
/* Returns the next client in restack avaible.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure. 
 */
Client *nextrstack(Client *c);
/* Returns the next client in focus order avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nextfocus(Client *c);
Client *nexttiled(Client *c);
/* Returns the next visible client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nextvisible(Client *c);
/* Returns the previous desktop avaible. 
 * RETURN: Desktop * on Success.
 * RETURN: NULL on Failure.
 */
Desktop *prevdesktop(Desktop *desk);
/* Returns the prev client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *prevclient(Client *c);
/* Returns the prev focus client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *prevfocus(Client *c);
/* Returns the prev stack client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *prevstack(Client *c);
/* Returns the previous restack stack client avaible.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *prevrstack(Client *c);
/* Returns the prev visible client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *prevvisible(Client *c);
/* Sends a event to the main event loop to stop running.
 */
void quit(void);
/* Attempts to restore session from SESSION_FILE for all monitors */
void restoresession(void);
/* Attempts to restore session from buff for a client */
Client *restoreclientsession(Desktop *desk, char *buff, uint16_t len);
/* Attempts to restore session from buff for a desktop */
Desktop *restoredesktopsession(Monitor *m, char *buff, uint16_t len);
/* Attempts to restore session from buff for given monitor */
Monitor *restoremonsession(char *buff, uint16_t len);
/* Searches through every monitor for a possible big enough size to fit rectangle parametors specified */
Monitor *recttomon(int16_t x, int16_t y, uint16_t width, uint16_t height);
/* resize a client only if specified x/y/w/h is different 
 * interact
 * {1, 0}
 * 1 -> dont confide resize to monitor dimentions 
 * 0 -> confide resize within monitor dimentions
 * */
void resize(Client *c, int32_t x, int32_t y, int32_t width, int32_t height, uint8_t interact);
/* resize a client given parametors without sizehints */
void resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height);
/* Reorders(restacks) clients in current desk->stack */
void restack(Desktop *desk);
/* "Restacks" clients on from linked list no effect unless restack called*/
void reorder(Desktop *desk);
/* Flags RESTART and sets running to 0;
 * results in execvp(self) and "restarts"
 */
void restart(void);
/* Main event loop */
void run(void);
/* Attemps to save session in for every monitor */
void savesession(void);
/* Attemps to save session for specified client. 
 */
void saveclientsession(FILE *fw, Client *c);
/* Attemps to save session for specified desktop */
void savedesktopsession(FILE *fw, Desktop *desktop);
/* Attemps to save session from file for specified Monitor 
 */
void savemonsession(FILE *fw, Monitor *m);
/* Scans for new clients on startup */
void scan(void);
/* Sends a Protocol Event to specified client */
void sendprotocolevent(Client *c, XCBAtom proto);
/* Sets the flag "alwaysontop" to the provided Client */
void setalwaysontop(Client *c, uint8_t isalwaysontop);
/* Sets the flag "alwaysonbottom" to the provided Client */
void setalwaysonbottom(Client *c, uint8_t state);
/* Sets the Clients border opacity, "alpha" 0-255 */
void setborderalpha(Client *c, uint8_t alpha);
/* Sets the border color using red green and blue values */
void setbordercolor(Client *c, uint8_t red, uint8_t green, uint8_t blue);
/* Sets the border color only using the 32bit value */
void setbordercolor32(Client *c, uint32_t col);
/* Sets the border width to the provided Client */
void setborderwidth(Client *c, uint16_t border_width);
/* Sets the clients desktop to the specified desktop, 
 * and cleanups any data that may have been left from the previous desktop. 
 */
void setclientdesktop(Client *c, Desktop *desktop);
/* Sets the clients wmatom[WMState] property. */
void setclientstate(Client *c, uint8_t state);
/* Sets the decor visibility. */
void setdecorvisible(Client *c, uint8_t state);
/* Sets the desktop count rolling back any clients to previous desktops. */
void setdesktopcount(Monitor *m, uint16_t desktops);
/* Sets the desktops layouts, (not automatic arrange must be called after to apply changes.) */
void setdesktoplayout(Desktop *desk, uint8_t layout);
/* Sets the clients pid. */
void setclientpid(Client *c, pid_t pid);
/* Sets the Clients IS Desktop Flag. */
void setwtypedesktop(Client *c, uint8_t state);
/* Sets the Clients IS Dialog Flag. */
void setwtypedialog(Client *c, uint8_t state);
/* Sets the Clients IS Dock Flag. */
void setwtypedock(Client *c, uint8_t state);
/* Sets the Clients IS ToolBar Flag. */
void setwtypetoolbar(Client *c, uint8_t state);
/* Sets the Clients IS Menu Flag. */
void setwtypemenu(Client *c, uint8_t state);
/* Sets the Clients Never Focus Flag. */
void setwtypeneverfocus(Client *c, uint8_t state);
/* Sets the Clients IS Utility Flag. */
void setwtypeutility(Client *c, uint8_t state);
/* Sets the Clients IS Splash Flag. */
void setwtypesplash(Client *c, uint8_t state);
/* Sets the Clients IS Dropdown Menu Flag. */
void setwtypedropdownmenu(Client *c, uint8_t state);
/* Sets the Clients IS Popup Menu Flag. */
void setwtypepopupmenu(Client *c, uint8_t state);
/* Sets the Clients IS Tool Tip Flag. */
void setwtypetooltip(Client *c, uint8_t state);
/* Sets the Clients IS Notification Flag. */
void setwtypenotification(Client *c, uint8_t state);
/* Sets the Clients IS Combo Flag. */
void setwtypecombo(Client *c, uint8_t state);
/* Sets the Clients IS DND Flag. */
void setwtypednd(Client *c, uint8_t state);
/* Sets the Clients IS Normal Flag. */
void setwtypenormal(Client *c, uint8_t state);
/* Sets the Clients IS Map Iconic Flag. */
void setwtypemapiconic(Client *c, uint8_t state);
/* Sets the Clients IS Map Normal Flag. */
void setwtypemapnormal(Client *c, uint8_t state);
/* Sets the Clients IS Take Focus Flag. */
void setwmtakefocus(Client *c, uint8_t state);
/* Sets the Clients IS Save Yourself Flag. */
void setwmsaveyourself(Client *c, uint8_t state);
/* Sets the Clients IS Delete Window Flag. */
void setwmdeletewindow(Client *c, uint8_t state);
/* Sets the Clients IS Skip Pager Flag. */
void setskippager(Client *c, uint8_t state);
/* Sets the Clients IS Skip Taskbar Flag. */
void setskiptaskbar(Client *c, uint8_t state);
/* Sets the Clients Show Decor Flag. */
void setshowdecor(Client *c, uint8_t state);
/* Makes a client fullscreen and take up the entire monitor. (also sets the isfullscreen flag)*/
void setfullscreen(Client *c, uint8_t isfullscreen);
/* Sets the Clients IS Floating Flag. */
void setfloating(Client *c, uint8_t state);
/* Sets the current Window Focus. */
void setfocus(Client *c);
/* Sets the Windows Map State (Iconic/Normal), and IS hidden Flag. */
void sethidden(Client *c, uint8_t state);
/* Sets the "Maximized Vert" Flag */
void setmaximizedvert(Client *c, uint8_t state);
/* Sets the "Maximized Horz" Flag */
void setmaximizedhorz(Client *c, uint8_t state);
/* Sets the Clients IS Shaded Flag. */
void setshaded(Client *c, uint8_t state);
/* Sets the Clients IS Modal Flag. */
void setmodal(Client *c, uint8_t state);
/* Sets the monitors currently selected desktop. */
void setmondesktop(Monitor *m, Desktop *desk);
/* Replaces the Clients state with the sticky state, and sets IS sticky Flag. */
void setsticky(Client *c, uint8_t state);
/* Sets the title bar to be on top. */
void settopbar(Client *c, uint8_t state);
/* Vital checks and data setup before any other action is performed. */
void startup(void);
/* Sets up Variables, Checks, WM specific data, etc.. */
void setup(void);
/* Sets up the cursors used for the WM. */
void setupcursors(void);
/* Loads CFG data into Settings struct. */
void setupcfg(void);
/* Loads default if CFG data failed to read. */
void setupcfgdefaults(void);
/* Updates a Clients state to Urgent, and sets the Urgent Flag. (Updates window border to urgen color.) */
void seturgent(Client *c, uint8_t isurgent);
/* Moves Client offscreen if not VISIBLE;
 * Moves Client onscreen if VISIBLE;
 */
void showhide(Client *c);
/* waits for childs (zombies) to die */
void sigchld(int signo);
/* Handles Signals and how we use them */
void sighandler(void);
/* Calls restart */
void sighup(int signo);
/* Calls quit */
void sigterm(int signo);
/* Error checks, and checks for certain conditions to be meet for different behaviour 
 * NOTE: This is only checked when the program is about to exit.
 */
void specialconds(int argc, char *argcv[]);
/* Setups most vital data for the context. 
 * Calls exit(1) on Failure.
 */
void startup(void);
/* Sets the "Tiled" layout for the specified desktop.
 * Tiled -> Windows tile in a grid like patter where there is 1 Big window to the left,
 *          and "stacking" on top of each other smaller windows on the right.
 */
void tile(Desktop *desk);
/* Unfocuses specified client and sets to focus to root if setfocus is true */
void unfocus(Client *c, uint8_t setfocus);
/* updates the Status Bar Position from given monitor */
void updatebarpos(Monitor *m);
/* updates the bar geometry from the given monitor */
void updatebargeom(Monitor *m);
/* updates a clients classname from XCBWMClass *_class 
 * No side effects on non filled _class dataw;
 */
void updateclass(Client *c, XCBWMClass *_class);
/* Updates 
 * type:            0       Adds the client win .
 *                  1       Removes the specified win.
 *                  2       Reloads the entire list.
 * _NET_WM_CLIENT_LIST */
void updateclientlist(XCBWindow win, uint8_t type);
void updatedecor(Client *c);
/* Updates the XServer to the Current destop */
void updatedesktop(void);
/* Updates the desktop names if they have changed */
void updatedesktopnames(void);
/* Updates the current desktop count AKA how many desktops we got to the XServer */
void updatedesktopnum(void);
/* Updates Geometry for external monitors based on if they have different geometry */
int  updategeom(void);
/* Updates the Client icon if we find one */
void updateicon(Client *c, XCBWindowProperty *iconprop);
/* TODO */
void updatemotifhints(void);
/* checks and updates mask if numlock is active */
void updatenumlockmask(void);
/* Updates a Clients sizehints property using the provided hints pointer "size".
 * Doesnt require any data from client, AKA modular. still requires "size" though.
 */
void updatesizehints(Client *c, XCBSizeHints *size);
/* Updates Client tile if we find one;
 * if none found default to dwm.h BROKEN
 */
void updatetitle(Client *c, char *netwmname, char *wmname);
/* updates the viewport property to the XServer */
void updateviewport(void);
/* Updates Our own window protocol status (dont have to query every time) */
void updatewindowprotocol(Client *c, XCBWMProtocols *protocols);
/* Updates Our own state based on Client state specified */
void updatewindowstate(Client *c, XCBAtom state, uint8_t add_remove_toggle);
/* Updates Our own states based on Client state specified */
void updatewindowstates(Client *c, XCBAtom state[], uint32_t atomslength);
/* Updates Our own state based on windowtype in Client */
void updatewindowtype(Client *c, XCBAtom wtype, uint8_t add_remove_toggle);
/* Updates Our own states based on windowtype in Client */
void updatewindowtypes(Client *c, XCBAtom wtype[], uint32_t atomslength);
/* Updates WM_HINTS for specified Client */
void updatewmhints(Client *c, XCBWMHints *hints);
/* Wakups the current X connection by sending a event to it */
void wakeupconnection(XCBDisplay *display, int screen);
/* Sets the window _WM_STATE based on state specified */
void winsetstate(XCBWindow win, int32_t state);
/* Returns the bar or the monitor depending on if is_return_mon is true.
 * RETURN: Bar * on Success and is_return_mon is set to False.
 * RETURN: Monitor * on Success and is_return_mon is set to True.
 * RETURN: NULL on Failure.
 */
void *wintobar(XCBWindow win, uint8_t is_return_mon);
/* Returns the client if found from the specified window 
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *wintoclient(XCBWindow win);
/* Returns the Monitor if found from the specified window 
 * RETURN: Monitor* on Success.
 * RETURN: NULL on Failure.
 */
Monitor *wintomon(XCBWindow win);
/* Unmanages Client AKA we dont tell it what todo Nor does it use our resources;
 * And perform checks based on specified "destroyed";
 * 1 -> widthdraw window;
 * 0 -> skip checks (window already destroyed)
 */
void unmanage(Client *c, uint8_t destroyed);
/* memsets the specified bar to all 0's thus unmanaging the bar
 */
void unmanagebar(Bar *bar);
/* unmaximizes a client if maxed, Sets flag. */
void unmaximize(Client *c);
/* unmaximizes a client horizontally if maxed horz, Sets flag. */
void unmaximizehorz(Client *c);
/* unmaximizes a client vertically if maxed vert, Sets flag. */
void unmaximizevert(Client *c);
/* Error handler */
void xerror(XCBDisplay *display, XCBGenericError *error);


/* MACROS */

int ISALWAYSONTOP(Client *c);
int ISALWAYSONBOTTOM(Client *c);
int WASFLOATING(Client *c);
int ISFLOATING(Client *c);
int ISFAKEFLOATING(Client *c);
int DOCKEDVERT(Client *c);
int DOCKEDHORZ(Client *c);
int DOCKED(Client *c);
int WASDOCKEDVERT(Client *c);
int WASDOCKEDHORZ(Client *c);
int WASDOCKED(Client *c);
int ISFIXED(Client *c);
int ISURGENT(Client *c);
int NEVERFOCUS(Client *c);
int ISVISIBLE(Client *c);
int SHOWDECOR(Client *c);
int ISSELECTED(Client *c);
/* checks if a client could be a bar */
int COULDBEBAR(Client *c, uint8_t strut);

/* EWMH Window types */
int ISDESKTOP(Client *c);
int ISDOCK(Client *c);
int ISTOOLBAR(Client *c);
int ISMENU(Client *c);
int ISUTILITY(Client *c);
int ISSPLASH(Client *c);
int ISDIALOG(Client *c);
int ISDROPDOWNMENU(Client *c);
int ISPOPUPMENU(Client *c);
int ISTOOLTIP(Client *c);
int ISNOTIFICATION(Client *c);
int ISCOMBO(Client *c);
int ISDND(Client *c);
int ISNORMAL(Client *c);
int ISMAPICONIC(Client *c);
int ISMAPNORMAL(Client *c);
/* EWMH Window states */
int ISMODAL(Client *c);
int ISSTICKY(Client *c);
int ISMAXIMIZEDVERT(Client *c);
int ISMAXIMIZEDHORZ(Client *c);
int ISSHADED(Client *c);
int SKIPTASKBAR(Client *c);
int SKIPPAGER(Client *c);
int ISHIDDEN(Client *c);
int ISFULLSCREEN(Client *c);
int ISABOVE(Client *c);
int ISBELOW(Client *c);
int DEMANDSATTENTION(Client *c);
int ISFOCUSED(Client *c);
/* WM Protocol */
int HASWMTAKEFOCUS(Client *c);
int HASWMSAVEYOURSELF(Client *c);
int HASWMDELETEWINDOW(Client *c);

enum BarSides GETBARSIDE(Monitor *m, Bar *bar);

uint16_t OLDWIDTH(Client *c);
uint16_t OLDHEIGHT(Client *c);
uint16_t WIDTH(Client *c);
uint16_t HEIGHT(Client *c);


static const Layout layouts[LayoutTypeLAST] =
{
    /* Name             arrange     */
    [Tiled]     = {     tile        },
    [Floating]  = {     floating    },
    [Monocle]   = {     monocle     },
    [Grid]      = {     grid        },
};

#endif 
