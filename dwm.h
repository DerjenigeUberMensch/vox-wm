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
#define CLEANMASK(mask)         (mask & ~(numlockmask|XCB_MOD_MASK_LOCK) & \
                                (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL| \
                                 XCB_MOD_MASK_1|XCB_MOD_MASK_2|XCB_MOD_MASK_3|XCB_MOD_MASK_4|XCB_MOD_MASK_5))
#define MOUSEMASK               (BUTTONMASK | XCB_EVENT_MASK_POINTER_MOTION)
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TAGSLENGTH              (LENGTH(tags))

#define SESSION_FILE            "/tmp/dwm-session"

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
#define _OSHOWBAR           ((1 << 2))
#define _TOPBAR             ((1 << 3))


/* Client macros */

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
typedef struct Tag Tag;
typedef struct Client Client;
typedef struct Layout Layout;

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
    char name[256];     /* Client Name              */
    float mina;         /* Minimum Aspect           */
    float maxa;         /* Maximum Aspect           */
    int16_t x;          /* X coordinate             */
    int16_t y;          /* Y coordinate             */
    uint16_t w;         /* Width                    */
    uint16_t h;         /* height                   */
    int16_t oldx;       /* Previous X coordinate    */
    int16_t oldy;       /* Previous Y coordinate    */
    uint16_t oldw;      /* Previous Width           */
    uint16_t oldh;      /* Previous Height          */
    uint16_t basew;     /* Base Width               */
    uint16_t baseh;     /* Base Height              */
    int16_t incw;       /* Increment Width          */
    int16_t inch;       /* Increment Height         */
    uint16_t maxw;      /* Max Width                */
    uint16_t maxh;      /* Max Height               */
    uint16_t minw;      /* Minimum Width            */
    uint16_t minh;      /* Minimum Height           */

    uint16_t bw;        /* Border Width             */
    uint16_t oldbw;     /* Old Border Width         */

    /* XXX See Monitor seltags (assuming you change this to uint32_t ) */
    uint16_t tags;      /* Tags for clients base 2;
                         * 2^x;
                         * {1, 2, 4, 8, 16, 32, 64, 128...}
                         */
    uint16_t flags;     /* Flags for client         */

    pid_t pid;          /* Client Pid               */
    XCBWindow win;      /* Client Window            */
    Monitor *mon;       /* Client Monitor           */
};

struct Layout
{
    const char *symbol;
    void (*arrange)(Monitor *);
};

struct Monitor
{
    char ltsymbol[16];          /* Layout symbol                            */
    int16_t mx;                 /* Monitor X (Screen Area)                  */
    int16_t my;                 /* Monitor Y (Screen Area)                  */
    int16_t mw;                 /* Monitor Width (Screen Area)              */
    uint16_t mh;                /* Monitor Height (Screen Area)             */
    int16_t wx;                 /* Monitor X (Window Area)                  */
    int16_t wy;                 /* Monitor Y (Window Area)                  */
    uint16_t ww;                /* Monitor Width (Window Area)              */
    uint16_t wh;                /* Monitor Height (Window Area)             */

    uint16_t flags;             /* Monitor flags                            */


    uint16_t tag;               /* Current tag in view                      */
    uint16_t otag;              /* Previous (Old) tag in view               */
    uint8_t  layout;            /* The Layout Index that is currently used  */
    uint8_t  olayout;           /* The Previous (Old) Layout Index          */

    Client *clients;            /* First Client in linked list              */
    Client *stack;              /* First Stack Client in linked list        */
    Client *sel;                /* Selected Client                          */
    Monitor *next;              /* Next Monitor                             */

    XCBWindow barwin;           /* The managed status bar                   */
};





void checkotherwm(void);
void cleanup(void);
void grabbuttons(XCBWindow window, int focused);
void grabkeys(void);
void run(void);
void scan(void);
void setup(void);
void sigchld(int signo);
void sighandler(void);
void sighup(int signo);
void sigterm(int signo);
void startup(void);
void updatenumlockmask(void);






#endif 
