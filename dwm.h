#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <stdio.h>

#include "uthash.h"
#include "settings.h"
#include "pannel.h"
#include "client.h"
#include "XCB-TRL/xcb_trl.h"
#include "XCB-TRL/xcb_winutil.h"
#include "XCB-TRL/xcb_gtk.h"


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
typedef struct Decoration Decoration;
typedef struct Stack Stack;
typedef struct Layout Layout;
typedef struct Desktop Desktop;
typedef struct WM WM;
typedef struct MotifWmHints MotifWmHints;

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
    Client *bar;                /* The Associated Task-Bar (can be NULL)    */
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

struct MotifWmHints
{
    /* These correspond to XmRInt resources. (VendorSE.c) */
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    uint32_t input_mode;
    uint32_t status;
};


/* Handles the main(int argc, char **argv) arguments. */
void argcvhandler(int argc, char *argv[]);
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
 * if it is then it becomes the new bar.
 * RETURN: 0 on Success.
 * RETURN: 1 on no new bar (Failure).
*/
uint8_t checknewbar(Monitor *m, Client *c, const uint8_t has_strut_or_strut_partial);
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
/* Allocates memory and resturns the pointer in **str_return from the specified XCBWindowProperty. */
void getnamefromreply(XCBWindowProperty *namerep, char **str_return);
/* Gets the icon property from the specified XCBWindowProperty. */
uint32_t *geticonprop(XCBWindowProperty *iconreply);
/* Sets the "grid" layout for the specified desktop.
 * grid -> windows are sorted in a grid like formation, like those ones in hacker movies.
 */
void grid(Desktop *desk);
/* Sets the "monocle" layout for the specified desktop.
 * monocle -> Windows are maximized to the screen avaible area, 
 * while floating windows are always raised above all others.
 */
void monocle(Desktop *desk);
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
/* Returns the previous desktop avaible. 
 * RETURN: Desktop * on Success.
 * RETURN: NULL on Failure.
 */
Desktop *prevdesktop(Desktop *desk);
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
/* Vital checks and data setup before any other action is performed. */
void startup(void);
/* Sets up Variables, Checks, WM specific data, etc.. */
void setup(void);
/* Sets up Atoms ID's from the XServer */
void setupatoms(void);
/* Sets up special data. */
void setupbar(Monitor *m, Client *bar);
/* Sets up the cursors used for the WM. */
void setupcursors(void);
/* Loads CFG data into Settings struct. */
void setupcfg(void);
/* Loads default if CFG data failed to read. */
void setupcfgdefaults(void);
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
/*      reference point is c1.
 *      so if c1 has higher priority return 1.
 *      RETURN: 1 on higher priority.
 *      RETURN: 0 on lesser priority.
 */
int stackpriority(Client *c1, Client *c2);
/* Setups most vital data for the context. 
 * Calls exit(1) on Failure.
 */
void startup(void);
/* Sets the "Tiled" layout for the specified desktop.
 * Tiled -> Windows tile in a grid like patter where there is 1 Big window to the left,
 *          and "stacking" on top of each other smaller windows on the right.
 */
void tile(Desktop *desk);
/* updates the Status Bar Position from given monitor */
void updatebarpos(Monitor *m);
/* updates the bar geometry from the given monitor */
void updatebargeom(Monitor *m);
/* Updates 
 * type:            0       Adds the client win .
 *                  1       Removes the specified win.
 *                  2       Reloads the entire list.
 * _NET_WM_CLIENT_LIST */
void updateclientlist(XCBWindow win, uint8_t type);
/* Updates the XServer to the Current destop */
void updatedesktop(void);
/* Updates the desktop names if they have changed */
void updatedesktopnames(void);
/* Updates the current desktop count AKA how many desktops we got to the XServer */
void updatedesktopnum(void);
/* Updates Geometry for external monitors based on if they have different geometry */
int  updategeom(void);
/* checks and updates mask if numlock is active */
void updatenumlockmask(void);
void updatestackpriorityfocus(Desktop *desk);
/* updates the viewport property to the XServer */
void updateviewport(void);
/* Wakups the current X connection by sending a event to it */
void wakeupconnection(XCBDisplay *display, int screen);
/* Returns the Monitor if found from the specified window 
 * RETURN: Monitor* on Success.
 * RETURN: NULL on Failure.
 */
Monitor *wintomon(XCBWindow win);
/* Error handler */
void xerror(XCBDisplay *display, XCBGenericError *error);

/* checks if a client could be a bar */
int COULDBEBAR(Client *c, uint8_t strut);

enum BarSides GETBARSIDE(Monitor *m, Client *bar, uint8_t get_prev_side);


static const Layout layouts[LayoutTypeLAST] =
{
    /* Name             arrange     */
    [Tiled]     = {     tile        },
    [Floating]  = {     floating    },
    [Monocle]   = {     monocle     },
    [Grid]      = {     grid        },
};

#endif 
