#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

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
#define _ALWAYSONTOP        ((1 << 0))
#define _FULLSCREEN         ((1 << 1))
#define _WASFLOATING        ((1 << 2))
#define _FLOATING           ((1 << 3))
#define _FIXED              ((1 << 4))
#define _URGENT             ((1 << 5))
#define _NEVERFOCUS         ((1 << 6))
#define _HIDDEN             ((1 << 7))
#define _STICKY             ((1 << 8))
#define _DIALOG             ((1 << 9))
#define _MODAL              ((1 << 10))

/* Client macros */

/* This returns non zero on true, but not necessarly 1 */
#define ISALWAYSONTOP(C)        (((C)->flags & _ALWAYSONTOP))
/* This returns non zero on true, but not necessarly 1 */
#define ISFULLSCREEN(C)         (((C)->flags & _FULLSCREEN))
/* This returns non zero on true, but not necessarly 1 */
#define WASFLOATING(C)          (((C)->flags & _WASFLOATING))
/* This returns non zero on true, but not necessarly 1 */
#define ISFLOATING(C)           (((C)->flags & _WASFLOATING))
/* This returns non zero on true, but not necessarly 1 */
#define ISFIXED(C)              (((C)->flags & _FIXED))
/* This returns non zero on true, but not necessarly 1 */
#define ISURGENT(C)             (((C)->flags & _URGENT))
/* This returns non zero on true, but not necessarly 1 */
#define NEVERFOCUS(C)           (((C)->flags & _NEVERFOCUS))
/* This returns non zero on true, but not necessarly 1 */
#define ISHIDDEN(C)             (((C)->flags & _HIDDEN))
/* This returns non zero on true, but not necessarly 1 */
#define ISSTICKY(C)             (((C)->flags & _STICKY))
/* This returns non zero on true, but not necessarly 1 */
#define ISDIALOG(C)             (((C)->flags & _DIALOG))
/* This returns non zero on true, but not necessarly 1 */
#define ISMODAL(C)              (((C)->flags & _MODAL))
/* This returns 1 when true */
#define ISVISIBLE(C)            ((((C)->mon->desksel == (C)->desktop || ISSTICKY(c)) & (!ISHIDDEN(c))))

/* Monitor struct flags */
#define _SHOWBAR            ((1 << 0))
#define _OSHOWBAR           ((1 << 1))
#define _TOPBAR             ((1 << 2))

/* Monitor Macros */

/* This returns non zero on true, but not necessarly 1 */
#define SHOWBAR(M)              (((M)->flags & _SHOWBAR))
/* This returns non zero on true, but not necessarly 1 */
#define OSHOWBAR(M)             (((M)->flags & _OSHOWBAR))
/* This returns non zero on true, but not necessarly 1 */
#define TOPBAR(M)               (((M)->flags & _TOPBAR))


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

    uint16_t flags;     /* Flags for client         */
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

    uint8_t pad0[16];
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

    uint16_t flags;             /* Monitor flags                            */
    uint16_t deskcount;         /* Desktop Counter                          */

    Desktop *desktops;          /* First Desktop in linked list             */
    Desktop *desksel;           /* Selected Desktop                         */
    Monitor *next;              /* Next Monitor                             */

    int16_t bx;                 /* Bar X                                    */
    int16_t by;                 /* Bar Y                                    */
    uint16_t bw;                /* Bar Width                                */
    uint16_t bh;                /* Bar Height                               */
    XCBWindow root;             /* The Root window                          */
    XCBWindow barwin;           /* The managed status bar                   */
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
    Client *slast;              /* Last client in stack order   */
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
void attachbar(Monitor *m, XCBWindow barwin);
void detachbar(Monitor *m);
void attachdesktop(Monitor *m, Desktop *desk);
void detachdesktop(Monitor *m, Desktop *desk);
void attach(Client *c);
void attachstack(Client *c);
void detach(Client *c);
void detachcompletely(Client *c);
void detachstack(Client *c);
void cfgsethoverfocus(Client *c, uint8_t state);
uint8_t checknewbar(XCBWindow win);
void checkotherwm(void);
void cleanup(void);
void cleanupclient(Client *c);
void cleanupdesktop(Desktop *desk);
void cleanupmon(Monitor *m);
void cleanupmons(void);
void configure(Client *c);
Client *createclient(Monitor *m);
Desktop *createdeskop(Monitor *m);
Monitor *createmon(void);
Stack *createstack(void);
Monitor *dirtomon(uint8_t dir);
uint8_t docked(Client *c);
void eventhandler(XCBGenericEvent *ev);
void exithandler(void);
void floating(Desktop *desk);
void focus(Client *c);
int32_t getstate(XCBWindow win);
void grabbuttons(XCBWindow window, uint8_t focused);
void grabkeys(void);
void grid(Desktop *desk);
Client *manage(XCBWindow window);
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
void setdialog(Client *c, uint8_t state);
void setfixed(Client *c, uint8_t state);
void setfloating(Client *c, uint8_t isfloating);
void setfullscreen(Client *c, uint8_t isfullscreen);
void setfocus(Client *c);
void sethidden(Client *c, uint8_t state);
void setmodal(Client *c, uint8_t state);
void setneverfocus(Client *c, uint8_t state);
void setshowbar(Monitor *m, uint8_t state);
void setsticky(Client *c, uint8_t state);
void settopbar(Monitor *m, uint8_t state);
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
void updateclientlist(void);
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
Client *wintoclient(XCBWindow win);
Monitor *wintomon(XCBWindow win);


void unmanage(Client *c, uint8_t destroyed);

void xerror(XCBDisplay *display, XCBGenericError *error);

/* toogle */
void UserStats(const Arg *arg);



#endif 
