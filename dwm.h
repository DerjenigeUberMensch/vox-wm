#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "bar.h"
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
#define WASFLOATING(C)          (( ((C)->mon->wx == (C)->oldx) & ((C)->mon->wy == (C)->oldy) & ((C)->mon->ww == (C)->oldw) & ((C)->mon->wh == (C)->oldh) ))
#define ISFLOATING(C)           (( ((C)->mon->wx == (C)->x) & ((C)->mon->wy == (C)->y) & ((C)->mon->ww == (C)->w) & ((C)->mon->wh == (C)->h)  ))
#define ISFIXED(C)              (( ((C)->minw != 0 ) & ((C)->minh != 0) & ((C)->minw == (C)->maxw) & ((C)->minh == (C)->maxh) ))
#define ISURGENT(C)             (((C)->wstateflags & _STATE_DEMANDS_ATTENTION))
#define NEVERFOCUS(C)           (((C)->wstateflags & _STATE_NEVERFOCUS))
#define ISMAXVERT(C)            (((C)->h == (C)->mon->wh))
#define ISMAXHORZ(C)            (((C)->w == (C)->mon->ww))
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
#define SKIPAGER(C)             (((C)->wstateflags & _STATE_SKIP_PAGER))
#define ISHIDDEN(C)             (((C)->wstateflags & _STATE_HIDDEN))
#define ISFULLSCREEN(C)         (((C)->wstateflags & _STATE_FULLSCREEN))
#define ISABOVE(C)              (((C)->wstateflags & _STATE_ABOVE))
#define ISBELOW(C)              (((C)->wstateflags & _STATE_BELOW))
#define DEMANDSATTENTION(C)     (((C)->wstateflags & _STATE_DEMANDS_ATTENTION))
#define ISFOCUSED(C)              (((C)->wstateflags & _STATE_FOCUSED))


/* This returns 1 when true */
#define ISVISIBLE(C)            ((((C)->mon->desksel == (C)->desktop || ISSTICKY((C))) & (!ISHIDDEN((C)))))

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



enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel }; /* color schemes */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

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
    uint8_t type;               /* KeyPress/KeyRelease  */
    uint16_t mod;               /* Modifier             */
    XCBKeysym keysym;           /* Key symbol           */
    void (*func)(const Arg *);  /* Function             */
    Arg arg;                    /* Argument             */
    uint8_t pad0[8];
};

struct Button
{
    uint16_t type;                  /* ButtonPress/ButtonRelease    */
    uint32_t mask;                  /* Modifier                     */
    uint32_t button;                /* Button                       */
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

    /* Client Flags */

    uint16_t wtypeflags;/* Window type flags        */
    uint16_t wstateflags;/* Window state flags      */

    XCBWindow win;      /* Client Window            */

    Client *next;       /* The next client in list  */
    Client *snext;      /* The next client in stack */
    Client *prev;       /* The previous client      */
    Client *sprev;      /* The prev stack order clnt*/
    Monitor *mon;       /* Client Monitor           */
    Desktop *desktop;   /* Client Associated Desktop*/

    uint16_t bw;        /* Border Width             */
    uint16_t oldbw;     /* Old Border Width         */

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

    pid_t pid;          /* Client Pid               */
    char *name;         /* Client Name              */
    char *icon;         /* Array of icon values     */

    uint8_t pad0[8];
};

struct Monitor
{
    int16_t mx;                 /* Monitor X (Screen Area)                  */
    int16_t my;                 /* Monitor Y (Screen Area)                  */
    int16_t mw;                 /* Monitor Width (Screen Area)              */
    uint16_t mh;                /* Monitor Height (Screen Area)             */
    int16_t wx;                 /* Monitor X (Window Area)                  */
    int16_t wy;                 /* Monitor Y (Window Area)                  */
    uint16_t ww;                /* Monitor Width (Window Area)              */
    uint16_t wh;                /* Monitor Height (Window Area)             */

    uint16_t deskcount;         /* Desktop Counter                          */

    Desktop *desktops;          /* First Desktop in linked list             */
    Desktop *desksel;           /* Selected Desktop                         */
    Monitor *next;              /* Next Monitor                             */

    Bar *bar;                   /* The Associated Task-Bar                  */
    uint8_t pad0[8];
};

struct Layout
{
    char *symbol;
    void (*arrange)(Desktop *);
    uint8_t pad0[16 + 32];      /* align for x64 */
};

struct Desktop
{
    int16_t num;                /* The Desktop Number           */
    uint8_t layout;             /* The Layout Index             */
    uint8_t olayout;            /* The Previous Layout Index    */

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
    int restart;                    /* Restart flag         */
    uint8_t has_error;              /* Error flag           */
    uint16_t sw;                    /* Screen Height u16    */
    uint16_t sh;                    /* Screen Width  u16    */
    XCBWindow root;                 /* The root window      */
    XCBWindow wmcheckwin;           /* window manager check */
    XCBDisplay *dpy;                /* The current display  */
    Monitor *selmon;                /* Selected Monitor     */
    Monitor *mons;                  /* Monitors             */
    XCBKeySymbols *syms;            /* keysym alloc         */
};

/* 
 * While flags would be more memory efficient they are a massive hastle
 */
struct CFG
{
    uint16_t nmaster;        /* number of clients in master area                                */
    uint16_t bw;             /* Default Border Width                                            */
    uint16_t bgw;            /* Default Border Gap Width                                        */

    uint16_t snap;           /* Window Resize/Move Snap to grid size                            */
    uint16_t rfrate;         /* max refresh rate when resizing, moving windows;  0 to disable   */

    uint16_t bh;             /* Bar Height.                                                     */
    uint16_t maxcc;          /* Max Client Count                                                */


    float mfact;
    char *wmname;
};

void argcvhandler(int argc, char *argv[]);
void applysizechecks(Monitor *m, int32_t *x, int32_t *y, int32_t *width, int32_t *height, int32_t *border_width);
void applygravity(uint32_t gravity, int16_t *x, int16_t *y, const uint16_t width, const uint16_t height, const uint16_t border_width);
uint8_t applysizehints(Client *c, int32_t *x, int32_t *y, int32_t *width, int32_t *height, uint8_t interact);
void arrange(Desktop *desk);
void arrangemon(Monitor *m);
void arrangemons(void);
void arrangedesktop(Desktop *desk);
void attachdesktop(Monitor *m, Desktop *desk);
void detachdesktop(Monitor *m, Desktop *desk);
void attach(Client *c);
void attachstack(Client *c);
void detach(Client *c);
void detachcompletely(Client *c);
void detachstack(Client *c);
void cfgsethoverfocus(Client *c, uint8_t state);
uint8_t checknewbar(int64_t strutpartial[12], XCBAtom windowtypes[], uint32_t windowtypeslength, XCBAtom windowstates[], uint32_t wmstateslength, int64_t desktop);
void checkotherwm(void);
void cleanup(void);
void cleanupclient(Client *c);
void cleanupdesktop(Desktop *desk);
void cleanupmon(Monitor *m);
void cleanupmons(void);
void configure(Client *c);
Client *createclient(Monitor *m);
Bar *createbar(void);
Desktop *createdeskop(Monitor *m);
Monitor *createmon(void);
Stack *createstack(void);
Monitor *dirtomon(uint8_t dir);
uint8_t docked(Client *c);
void eventhandler(XCBGenericEvent *ev);
void exithandler(void);
void floating(Desktop *desk);
void focus(Client *c);
int32_t getstate(XCBWindow win, XCBGetWindowAttributes *state);
void grabbuttons(XCBWindow window, uint8_t focused);
void grabkeys(void);
void grid(Desktop *desk);
Client *manage(XCBWindow window);
Bar *managebar(Monitor *m, XCBWindow win);
void monocle(Desktop *desk);
Client *nextclient(Client *c);
Desktop *nextdesktop(Desktop *desktop);
Monitor *nextmonitor(Monitor *monitor);
Client *nextstack(Client *c);
Client *nexttiled(Client *c);
Client *nextvisible(Client *c);
Client *lastvisible(Client *c);
void quit(void);
Monitor *recttomon(int16_t x, int16_t y, uint16_t width, uint16_t height);
void resize(Client *c, int32_t x, int32_t y, int32_t width, int32_t height, uint8_t interact);
void resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height);
void restack(Desktop *desk);
void restart(void);
void run(void);
void scan(void);
void setalwaysontop(Client *c, uint8_t isalwaysontop);
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
void setup(void);
void seturgent(Client *c, uint8_t isurgent);
void showhide(const Client *c);
void sigchld(int signo);
void sighandler(void);
void sighup(int signo);
void sigterm(int signo);
void specialconds(int argc, char *argcv[]);
void startup(void);
void tile(Desktop *desk);
void unfocus(Client *c, uint8_t setfocus);
void updatebarpos(Monitor *m);
void updatebargeom(Monitor *m);
void updateclientlist(void);
void updatedesktop(void);
void updatedesktopnames(void);
void updatedesktopnum(void);
int  updategeom(void);
void updateicon(Client *c);
void updatenumlockmask(void);
void updatesettings(void);
void updatesizehints(Client *c, XCBSizeHints *size);
void updatetitle(Client *c);
void updateviewport(void);
void updatewindowstate(Client *c, XCBAtom state, uint8_t add_remove_toggle);
void updatewindowstates(Client *c, XCBAtom state[], uint32_t atomslength);
void updatewindowtype(Client *c, XCBAtom wtype, uint8_t add_remove_toggle);
void updatewindowtypes(Client *c, XCBAtom wtype[], uint32_t atomslength);
void updatewmhints(Client *c, XCBWMHints *hints);
void winsetstate(XCBWindow win, int32_t state);
void *wintobar(XCBWindow win, uint8_t is_return_mon);
Client *wintoclient(XCBWindow win);
Monitor *wintomon(XCBWindow win);


void unmanage(Client *c, uint8_t destroyed);
void unmanagebar(Bar *bar);

void xerror(XCBDisplay *display, XCBGenericError *error);

/* toogle */
void UserStats(const Arg *arg);



#endif 
