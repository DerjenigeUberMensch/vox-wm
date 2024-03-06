#ifndef DWM_H
#define DWM_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "xcb_trl.h"
#include "xcb_winutil.h"


#ifndef VERSION
#define VERSION     "NOT_SET.NOT_SET.NOT_SET"
#endif

#define WM_NAME     "gamer"

#define BUTTONMASK              (XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE)
#define CLEANMASK(mask)         (mask & ~(_wm->numlockmask|XCB_MOD_MASK_LOCK) & \
                                (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL| \
                                 XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|XCB_MOD_MASK_4|XCB_MOD_MASK_5))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                                * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define MOUSEMASK               (BUTTONMASK | XCB_EVENT_MASK_POINTER_MOTION)
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

/* Monitor struct flags */
#define _SHOWBAR            ((1 << 0))
#define _OSHOWBAR           ((1 << 1))
#define _TOPBAR             ((1 << 2))


/* Client macros */

#define ISVISIBLE(C)            ((C->mon->desksel == C->desktop))
#define ISALWAYSONTOP(C)        ((C->flags & _ALWAYSONTOP))
#define ISFULLSCREEN(C)         ((C->flags & _FULLSCREEN))
#define WASFLOATING(C)          ((C->flags & _WASFLOATING))
#define ISFLOATING(C)           ((C->flags & _WASFLOATING))
#define ISFIXED(C)              ((C->flags & _FIXED))
#define ISURGENT(C)             ((C->flags & _URGENT))
#define NEVERFOCUS(C)           ((C->flags & _NEVERFOCUS))
#define ISHIDDEN(C)             ((C->flags & _HIDDEN))

/* Monitor Macros */

#define SHOWBAR(M)              ((M->flags & _SHOWBAR))
#define OSHOWBAR(M)             ((M->flags & _OSHOWBAR))
#define TOPBAR(M)               ((M->flags & _TOPBAR))



enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel }; /* color schemes */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union  Arg Arg;
typedef struct Key Key;
typedef struct Button Button;
typedef struct Monitor Monitor;
typedef struct Client Client;
typedef struct Layout Layout;
typedef struct Desktop Desktop;
typedef struct WM WM;

union Arg
{
    int i;              /* i -> int             */
    unsigned int ui;    /* ui -> unsigned int   */
    float f;            /* f -> float           */
    const void *v;      /* v -> void pointer    */
};

struct Key
{
    uint16_t type;              /* KeyPress/KeyRelease  */
    uint16_t mod;               /* Modifier             */
    XCBKeysym keysym;           /* Key symbol           */
    void (*func)(const Arg *);  /* Function             */
    const Arg arg;              /* Argument             */
};

struct Button
{
    uint16_t type;                  /* ButtonPress/ButtonRelease    */
    uint16_t click;                 /* enum ClkType                 */
    uint32_t mask;                  /* Modifier                     */
    uint32_t button;                /* Button                       */
    void (*func)(const Arg *arg);   /* Function                     */
    const Arg arg;                  /* Argument                     */
};

struct Client
{
    Client *next;       /* The next client in list  */
    Client *prev;       /* The previous client      */
    int16_t x;          /* X coordinate             */
    int16_t y;          /* Y coordinate             */
    uint16_t w;         /* Width                    */
    uint16_t h;         /* height                   */
    int16_t oldx;       /* Previous X coordinate    */
    int16_t oldy;       /* Previous Y coordinate    */
    uint16_t oldw;      /* Previous Width           */
    uint16_t oldh;      /* Previous Height          */

    uint16_t flags;     /* Flags for client         */

    uint16_t bw;        /* Border Width             */
    uint16_t oldbw;     /* Old Border Width         */

    XCBWindow win;      /* Client Window            */
    Monitor *mon;       /* Client Monitor           */
    Desktop *desktop;   /* Client Associated Desktop*/

    float mina;         /* Minimum Aspect           */
    float maxa;         /* Maximum Aspect           */
    uint16_t basew;     /* Base Width               */
    uint16_t baseh;     /* Base Height              */
    int16_t incw;       /* Increment Width          */
    int16_t inch;       /* Increment Height         */
    uint16_t maxw;      /* Max Width                */
    uint16_t maxh;      /* Max Height               */
    uint16_t minw;      /* Minimum Width            */
    uint16_t minh;      /* Minimum Height           */

    pid_t pid;          /* Client Pid               */

            /* +1 for \0 */
    char name[256 + 1]; /* Client Name              */

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

    Client *sel;                /* Selected Client                          */
    Desktop *desktops;          /* First Desktop in linked list             */
    Desktop *desksel;           /* Selected Desktop                         */
    Monitor *next;              /* Next Monitor                             */

    int16_t bx;                 /* Bar X                                    */
    int16_t by;                 /* Bar Y                                    */
    uint16_t bw;                /* Bar Width                                */
    uint16_t bh;                /* Bar Height                               */
    XCBWindow barwin;           /* The managed status bar                   */
};

struct Layout
{
    const char *symbol;
    void (*arrange)(Monitor *);
};

struct Desktop
{
    int16_t num;                /* The Desktop Number           */
    uint8_t layout;            /* The Layout Index             */
    uint8_t olayout;           /* The Previous Layout Index    */

    Client *clients;            /* First Client in linked list  */
    Desktop *next;              /* Next Client in linked list   */
    Desktop *prev;              /* Previous Client in list      */
};

/* WM
 * This struct does not use flags as it retains commonly used information that must be quickly accessible due to its vital nature */
struct WM
{
    int screen;                     /* Screen id            */
    uint16_t sw;                    /* Screen Height        */
    uint16_t sh;                    /* Screen Width         */
    XCBWindow root;                 /* The root window      */
    XCBWindow wmcheckwin;           /* window manager check */
    XCBDisplay *dpy;                /* The current display  */
    int numlockmask;                /* numlockmask          */
    Monitor *selmon;                /* Selected Monitor     */
    Monitor *mons;                  /* Monitors             */
    uint8_t running;                /* Running flag         */
    uint8_t restart;                /* Restart flag         */
    uint16_t default_layout;        /* default layout index */
    uint16_t desktopcount;          /* desktopcount         */
    char *msg;                      /* Message Buffer       */
    uint8_t has_error;              /* On true write msg buffer;
                                     * An prompt user to exit;
                                     */
    XCBKeySymbols *syms;            /* keysym alloc         */
};

void argcvhandler(int argc, char *argv[]);

void attachdesktop(Monitor *m, Desktop *desk);
void detachdesktop(Monitor *m, Desktop *desk);
void attachclient(Client *c);
void detachclient(Client *c);
void checkotherwm(void);

void cleanup(void);
void cleanupclient(Client *c);
void cleanupdesktop(Desktop *desk);
void cleanupmon(Monitor *m);
void cleanupmons(void);
Client *createclient(void);
Monitor *createmon(void);
void focus(Client *c);
void grabbuttons(XCBWindow window, uint8_t focused);
void grabkeys(void);
Client *nextvisible(Client *c);
void resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height);
void run(void);
void scan(void);
int sendevent(Client *c, XCBAtom proto);
void setalwaysontop(Client *c, uint8_t isalwaysontop);
void setborderwidth(Client *c, uint16_t border_width);
void setclientdesktop(Client *c, Desktop *desktop);
void setfloating(Client *c, uint8_t isfloating);
void setfullscreen(Client *c, uint8_t isfullscreen);
void setfocus(Client *c);
void setneverfocus(Client *c, uint8_t state);
void setup(void);
void seturgent(Client *c, uint8_t isurgent);
void sigchld(int signo);
void sighandler(void);
void sighup(int signo);
void sigterm(int signo);
void startup(void);
void unfocus(Client *c, uint8_t setfocus);
int  updategeom(void);
void updatenumlockmask(void);

Client *wintoclient(XCBWindow win);
Monitor *wintomon(XCBWindow win);






/* toogle */
void UserStats(const Arg *arg);




#endif 
