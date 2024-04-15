#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "bar.h"
#include "thread.h"
#include "xcb_trl.h"
#include "xcb_winutil.h"


#ifndef VERSION
#define VERSION     "NOT_SET.NOT_SET.NOT_SET"
#endif

#ifndef NAME
#define NAME        "NOT_SET"
#endif

#define BUTTONMASK              (XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE)
#define CLEANMASK(mask)         (mask & ~(_wm.numlockmask|XCB_MOD_MASK_LOCK) & \
                                (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL| \
                                 XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|XCB_MOD_MASK_4|XCB_MOD_MASK_5))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                                * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define MOUSEMASK               (BUTTONMASK | XCB_EVENT_MASK_POINTER_MOTION)
#define WIDTH(C)                ((C)->w + ((C)->bw << 1))
#define HEIGHT(C)               ((C)->h + ((C)->bw << 1))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TAGSLENGTH              (LENGTH(tags))
#define SESSION_FILE            "/tmp/dwm-session"
#define MAX_QUEUE_SIZE          1024
#define BORKED                  "NOT_SET"

/* Client struct flags */

/* Our custom states */

#define _NEVERFOCUS         ((1 << 6)

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

/* unused bits */
#define _TYPE_Y             ((1 << 14))
#define _TYPE_Z             ((1 << 15))

/* EWMH Window states */
#define _STATE_MODAL                ((1 << 0))
#define _STATE_STICKY               ((1 << 1))
#define _STATE_MAXIMIZED_VERT       ((1 << 2))  
#define _STATE_MAXIMIZED_HORZ       ((1 << 3))
#define _STATE_SHADED               ((1 << 4))
#define _STATE_SKIP_TASKBAR         ((1 << 5))
#define _STATE_SKIP_PAGER           ((1 << 6))
#define _STATE_HIDDEN               ((1 << 7))
#define _STATE_FULLSCREEN           ((1 << 8))
#define _STATE_ABOVE                ((1 << 9))
#define _STATE_BELOW                ((1 << 10))
#define _STATE_DEMANDS_ATTENTION    ((1 << 11))
#define _STATE_FOCUSED              ((1 << 12))
/* not actual state but just lumped in cause assinging its own is stupid. */
#define _STATE_NEVERFOCUS           ((1 << 13))

/* unused bits */
#define _STATE_Y                    ((1 << 14))
#define _STATE_Z                    ((1 << 15))


/* Client macros */

/* Our custom states */


#define ISALWAYSONTOP(C)        (((C)->wstateflags & _STATE_ABOVE))
#define ISALWAYSONBOTTOM(C)     (((C)->wstateflags & _STATE_BELOW))
#define WASFLOATING(C)          (( ((C)->desktop->mon->wx == (C)->oldx) & ((C)->desktop->mon->wy == (C)->oldy) & ((C)->desktop->mon->ww == (C)->oldw) & ((C)->desktop->mon->wh == (C)->oldh) ))
#define ISFLOATING(C)           (( ((C)->desktop->mon->wx == (C)->x) & ((C)->desktop->mon->wy == (C)->y) & ((C)->desktop->mon->ww == (C)->w) & ((C)->desktop->mon->wh == (C)->h)  ))
#define ISFIXED(C)              (( ((C)->minw != 0 ) & ((C)->minh != 0) & ((C)->minw == (C)->maxw) & ((C)->minh == (C)->maxh) ))
#define ISURGENT(C)             (((C)->wstateflags & _STATE_DEMANDS_ATTENTION))
#define NEVERFOCUS(C)           (((C)->wstateflags & _STATE_NEVERFOCUS))
#define ISMAXVERT(C)            (((C)->h == (C)->desktop->mon->wh))
#define ISMAXHORZ(C)            (((C)->w == (C)->desktop->mon->ww))
/* EWMH Window types */

#define ISDESKTOP(C)            (((C)->wtypeflags & _TYPE_DESKTOP))
#define ISDOCK(C)               (((C)->wtypeflags & _TYPE_DOCK))
#define ISTOOLBAR(C)            (((C)->wtypeflags & _TYPE_TOOLBAR))
#define ISMENU(C)               (((C)->wtypeflags & _TYPE_MENU))
#define ISUTILITY(C)            (((C)->wtypeflags & _TYPE_UTILITY))
#define ISSPLASH(C)             (((C)->wtypeflags & _TYPE_SPLASH))
#define ISDIALOG(C)             (((C)->wtypeflags & _TYPE_DIALOG))
#define ISDROPDOWNMENU(C)       (((C)->wtypeflags & _TYPE_DROPDOWN_MENU))
#define ISPOPUPMENU(C)          (((C)->wtypeflags & _TYPE_POPUP_MENU))
#define ISTOOLTIP(C)            (((C)->wtypeflags & _TYPE_TOOLTIP))
#define ISNOTIFICATION(C)       (((C)->wtypeflags & _TYPE_NOTIFICATION))
#define ISCOMBO(C)              (((C)->wtypeflags & _TYPE_COMBO))
#define ISDND(C)                (((C)->wtypeflags & _TYPE_DND))
#define ISNORMAL(C)             (((C)->wtypeflags & _TYPE_NORMAL))


/* EWMH Window states */

#define ISMODAL(C)              (((C)->wstateflags & _STATE_MODAL))
#define ISSTICKY(C)             (((C)->wstateflags & _STATE_STICKY))
#define ISMAXIMIZEDVERT(C)      (((C)->wstateflags & _STATE_MAXIMIZED_VERT))
#define ISMAXIMIZEDHORZ(C)      (((C)->wstateflags & _STATE_MAXIMIZED_HORZ))
#define ISSHADED(C)             (((C)->wstateflags & _STATE_SHADED))
#define SKIPTASKBAR(C)          (((C)->wstateflags & _STATE_SKIP_TASKBAR))
#define SKIPPAGER(C)             (((C)->wstateflags & _STATE_SKIP_PAGER))
#define ISHIDDEN(C)             (((C)->wstateflags & _STATE_HIDDEN))
#define ISFULLSCREEN(C)         (((C)->wstateflags & _STATE_FULLSCREEN))
#define ISABOVE(C)              (((C)->wstateflags & _STATE_ABOVE))
#define ISBELOW(C)              (((C)->wstateflags & _STATE_BELOW))
#define DEMANDSATTENTION(C)     (((C)->wstateflags & _STATE_DEMANDS_ATTENTION))
#define ISFOCUSED(C)              (((C)->wstateflags & _STATE_FOCUSED))


/* This returns 1 when true */
#define ISVISIBLE(C)            ((((C)->desktop->mon->desksel == (C)->desktop || ISSTICKY((C))) & (!ISHIDDEN((C)))))

/* Bar struct flags */

/* Bar Macros */

/* WM struct flags */
#define _CFG_HOVERFOCUS         ((1 << 0))
/*
#define _CFG_                   ((1 << 1))
#define _CFG_                   ((1 << 2))
#define _CFG_                   ((1 << 3))
#define _CFG_                   ((1 << 4))
#define _CFG_                   ((1 << 5))
#define _CFG_                   ((1 << 6))
#define _CFG_                   ((1 << 7))
*/

/* WM Macros */


#define CFG_ISHOVERFOCUS(G)     ((G)->flags & (_CFG_HOVERFOCUS))
/*
#define CFG(G)                  ((G)->flags & ())
#define CFG(G)                  ((G)->flags & ())
#define CFG(G)                  ((G)->flags & ())
*/


/* cursor */
enum CurType 
{ 
    CurNormal, 
    CurResize, 
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


typedef union  Arg Arg;
typedef struct Key Key;
typedef struct Button Button;
typedef struct Monitor Monitor;
typedef struct Client Client;
typedef struct Stack Stack;
typedef struct Layout Layout;
typedef struct Desktop Desktop;
typedef struct WM WM;
typedef struct CFG CFG;

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
    uint8_t pad0[8];
};

struct Button
{
    uint8_t type;                  /* ButtonPress/ButtonRelease    */
    uint8_t button;                /* Button                       */
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

    uint16_t bw;        /* Border Width             */
    uint16_t oldbw;     /* Old Border Width         */

    uint32_t bcol;      /* Border Colour            */

    float mina;         /* Minimum Aspect           */
    float maxa;         /* Maximum Aspect           */
    uint16_t basew;     /* Base Width               */
    uint16_t baseh;     /* Base Height              */
    uint16_t incw;      /* Increment Width          */
    uint16_t inch;      /* Increment Height         */
    uint16_t maxw;      /* Max Width                */
    uint16_t maxh;      /* Max Height               */
    uint16_t minw;      /* Minimum Width            */
    uint16_t minh;      /* Minimum Height           */

    XCBWindow win;      /* Client Window            */
    pid_t pid;          /* Client Pid               */

    Client *next;       /* The next client in list  */
    Client *snext;      /* The next client in stack */
    Client *prev;       /* The previous client      */
    Client *sprev;      /* The prev stack order clnt*/
    Desktop *desktop;   /* Client Associated Desktop*/

    wchar_t *name;      /* Client Name              */
    char *icon;         /* Array of icon values     */

    uint8_t pad0[8];
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
    Desktop *desksel;           /* Selected Desktop                         */
    Monitor *next;              /* Next Monitor                             */

    Bar *bar;                   /* The Associated Task-Bar                  */

    uint16_t deskcount;         /* Desktop Counter                          */
    uint8_t pad[2];
    uint8_t pad0[8];
};

struct Layout
{
    char *symbol;
    void (*arrange)(Desktop *);
};

struct Desktop
{
    int16_t num;                /* The Desktop Number           */
    
    uint8_t layout;             /* The Layout Index             */
    uint8_t olayout;            /* The Previous Layout Index    */

    Monitor *mon;               /* Desktop Monitor              */
    Client *lastfocused;        /* Last focused client          */
    Client *clients;            /* First Client in linked list  */
    Client *clast;              /* Last Client in linked list   */
    Client *stack;              /* Client Stack Order           */
    Client *slast;              /* Last Client in Stack         */
    Client *sel;                /* Selected Client              */
    Desktop *next;              /* Next Client in linked list   */
    Desktop *prev;              /* Previous Client in list      */
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
    Thread *ct;                     /* Current Thread       */
};

/* 
 * While flags would be more memory efficient they are a massive hastle
 */
struct CFG
{
    uint8_t  nmaster;           /* number of clients in master area                                 */
    uint8_t  hoverfocus;        /* 1 on mouse hover focus that window; 0 to disable                 */
    uint8_t  pad[2];

    uint16_t bw;                /* Border Width                                                     */
    uint16_t bgw;               /* Border Gap Width                                                 */

    uint32_t bcol;              /* Border Color                                                     */

    uint16_t snap;              /* Window Resize/Move Snap to grid size                             */
    uint16_t rfrate;            /* max refresh rate when resizing, moving windows;  0 to disable    */

    uint16_t bh;                /* Bar Height.                                                      */
    uint16_t maxcc;             /* Max Client Count                                                 */


    float mfact;
    char *wmname;
};

/* Handles the main(int argc, char **argv) arguments. */
void argcvhandler(int argc, char *argv[]);
/* Applies size restrictions to the specified values.
 * NOTE: Cannot handle any NULL values.
*/
void applysizechecks(Monitor *m, int32_t *x, int32_t *y, int32_t *width, int32_t *height, int32_t *border_width);
/* Applies the gravity shifts specified by the gravity onto the x and y coordinates.
*/
void applygravity(uint32_t gravity, int16_t *x, int16_t *y, const uint16_t width, const uint16_t height, const uint16_t border_width);
/* Applies size hints to the specified values.
* interact:             1/true/True         Does not restrict bounds to window area.
*                       0/false/False       Restricts bounds to window area.
* RETURN: 1 if the specified x/y/w/h does not match after sizehints applied. (aka need to resize.)
* RETURN: 0 if the specified x/y/w/h does match after the sizehints applied. (No need to resize.)
*/
uint8_t applysizehints(Client *c, int32_t *x, int32_t *y, int32_t *width, int32_t *height, uint8_t interact);
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
/* Checks given the provided information if a window is eligible to be a new bar.
 * RETURN: 1 on True.
 * RETURN: 0 on False
*/
uint8_t checknewbar(int64_t strutpartial[12], XCBAtom windowtypes[], uint32_t windowtypeslength, XCBAtom windowstates[], uint32_t wmstateslength, int64_t desktop);
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
/* frees client and allocated client properties. 
*/
void cleanupclient(Client *c);
/* frees desktop and allocated desktop properties.
*/
void cleanupdesktop(Desktop *desk);
/* frees Monitor and allocated Monitor properties.
*/
void cleanupmon(Monitor *m);
/* frees all monitors and allocated Monitor properties.
*/
void cleanupmons(void);
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
/* 
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
void killclient(XCBWindow win, enum KillType type);
/* Part of main event loop "run()"
 * Manages AKA adds the window to our current or windows specified desktop.
 * Applies size checks, bounds, layout, etc...
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *manage(XCBWindow window);
/* Sets the window to the specified bar, if the bar doesnt exist it creates it.
 * RETURN: Bar * on Success.
 * RETURN: NULL on Failure.
 */
Bar *managebar(Monitor *m, XCBWindow win);
/* Maximizes the speified clients vertical and horizontal axis.
 */
void maximize(Client *c);
/* Maximizes the speified clients horizontal axis.
 */
void maximizehorz(Client *c);
/* Maximizes the specified clients vertical axis.
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
/* Returns the next tiled client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nexttiled(Client *c);
/* Returns the next visible client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nextvisible(Client *c);
/* Returns the previous visible client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *lastvisible(Client *c);
/* Sends a event to the main event loop to stop running.
 */
void quit(void);
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
/* Flags RESTART and sets running to 0;
 * results in execvp(self) and "restarts"
 */
void restart(void);
/* Main event loop */
void run(void);
/* Scans for new clients on startup */
void scan(void);
/* Sends a Protocol Event to specified client */
uint8_t sendevent(XCBWindow win, XCBAtom proto);
/* Sets the flag "alwaysontop" to the provided Client */
void setalwaysontop(Client *c, uint8_t isalwaysontop);
void setborderalpha(Client *c, uint8_t alpha);
/* Sets the border color using red green and blue values */
void setbordercolor(Client *c, uint8_t red, uint8_t green, uint8_t blue);
/* Sets the border color only using the 32bit value */
void setbordercolor32(Client *c, uint32_t col);
/* Sets the border width to the provided Client */
void setborderwidth(Client *c, uint16_t border_width);
void setclientdesktop(Client *c, Desktop *desktop);
void setclientstate(Client *c, uint8_t state);
void setdesktoplayout(Desktop *desk, uint8_t layout);
void setwtypedialog(Client *c, uint8_t state);
void setfullscreen(Client *c, uint8_t isfullscreen);
void setfocus(Client *c);
void sethidden(Client *c, uint8_t state);
void setmodal(Client *c, uint8_t state);
void setneverfocus(Client *c, uint8_t state);
void setsticky(Client *c, uint8_t state);
void settopbar(Client *c, uint8_t state);
/* Sets up Variables, Checks, WM specific data, etc.. */
void setup(void);
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
/* Updates _NET_WM_CLIENT_LIST */
void updateclientlist(void);
/* Updates the XServer to the Current destop */
void updatedesktop(void);
/* Updates the desktop names if they have changed */
void updatedesktopnames(void);
/* Updates the current desktop count AKA how many desktops we got to the XServer */
void updatedesktopnum(void);
/* Updates Geometry for external monitors based on if they have different geometry */
int  updategeom(void);
/* Updates the Client icon if we find one */
void updateicon(Client *c);
/* checks and updates mask if numlock is active */
void updatenumlockmask(void);
/* Updates a Clients sizehints property using the provided hints pointer "size" */
void updatesizehints(Client *c, XCBSizeHints *size);
/* Updates Client tile if we find one;
 * if none found default to dwm.h BROKEN
 */
void updatetitle(Client *c);
/* updates the viewport property to the XServer */
void updateviewport(void);
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
void wakeupconnection();
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
/* Error handler */
void xerror(XCBDisplay *display, XCBGenericError *error);

#endif 
