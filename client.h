#ifndef _WM_CLIENT_H
#define _WM_CLIENT_H


#include "XCB-TRL/xcb_trl.h"
#include "uthash.h"

#include <stdint.h>

/* kill client type */
enum KillType 
{ 
    Graceful, 
    Safedestroy, 
    Destroy, 
};

/* Manage cookies */
enum ManageCookies
{
    ManageCookieAttributes,
    ManageCookieGeometry,
    ManageCookieTransient,
    ManageCookieWType,
    ManageCookieWState,
    ManageCookieSizeHint,
    ManageCookieWMHints,
    ManageCookieClass,
    ManageCookieWMProtocol,
    ManageCookieStrutP,
    ManageCookieStrut,
    ManageCookieNetWMName,
    ManageCookieWMName,
    ManageCookiePid,
    ManageCookieIcon,
    ManageCookieMotif,
    

    ManageCookieLAST
};

typedef struct Client Client;
typedef struct Decoration Decoration;

/* extern structs */
struct Desktop;

struct Decoration
{
    /* TODO */
    uint16_t w;
    uint16_t h;
    XCBWindow win;
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
    struct Desktop *desktop;   /* Client Associated Desktop*/
    Decoration *decor;  /* Decoration AKA title bar.*/

    char *netwmname;    /* Client Name              */
    char *wmname;       /* Client Name backup       */
    char *classname;    /* Class Name               */
    char *instancename; /* Instance Name            */
    uint32_t *icon;     /* Array of icon values     */

    UT_hash_handle hh;  /* hash handle              */
    uint16_t rstacknum; /* Used in calculating pos  */
    uint8_t pad[6];
};


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
/* Frees Client and allocated client properties. 
*/
void cleanupclient(Client *c);
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
/* Sets focus to the specified client.
 * NOTE: if NULL provided first visible client in stack is choosen as focus specifier.
 */
void focus(Client *c);
/* Grabs a win buttons. 
 * Basically this just allows us to receive button press/release events from windows.
 */
void grabbuttons(Client *c, uint8_t focused);
/* Grabs a windows keys.
 * Basically this just allows us to receive/intercept key press/release events.
 */
void grabkeys(void);
/* Kills the specified window.
 * type:            Graceful            Sends a message to the window to kill itself.
 *                  Safedestroy         Sends a message to the window to kill itself, on failure, forcefully kill the window.
 *                  Destroy             Destroys a window without sending any message for the window to response (Nuclear option.)
 */
void killclient(Client *c, enum KillType type);
/* requests for clients cookies. */
void managerequest(XCBWindow win, XCBCookie requests[ManageCookieLAST]);
/* Part of main event loop "run()"
 * Manages AKA adds the window to our current or windows specified desktop.
 * Applies size checks, bounds, layout, etc...
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *managereply(XCBWindow window, XCBCookie requests[ManageCookieLAST]);
/* Maximizes a client if unmaxed, Sets flag.
 */
void maximize(Client *c);
/* Maximizes horizontally a client if unmaxed horz, Sets flag.
 */
void maximizehorz(Client *c);
/* Maximizes vertically a client if unmaxed vert, Sets flag.
 */
void maximizevert(Client *c);
/* Returns the next client avaible.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *nextclient(Client *c);
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
/* Returns next tiled client when tiling.
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *nexttiled(Client *c);
/* Returns the next visible client avaible.
 * RETURN: Client* on Success.
 * RETURN: NULL on Failure.
 */
Client *nextvisible(Client *c);
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
/* resize a client only if specified x/y/w/h is different 
 * interact
 * {1, 0}
 * 1 -> dont confide resize to monitor dimentions 
 * 0 -> confide resize within monitor dimentions
 * */
void resize(Client *c, int32_t x, int32_t y, int32_t width, int32_t height, uint8_t interact);
/* resize a client given parametors without sizehints */
void resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height);

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
void setclientdesktop(Client *c, struct Desktop *desktop);
/* Sets the clients wmatom[WMState] property. */
void setclientstate(Client *c, uint8_t state);
/* Sets the decor visibility. */
void setdecorvisible(Client *c, uint8_t state);
/* Sets the flag to disable border >>CHANGES<< for a client. */
void setdisableborder(Client *c, uint8_t state);
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
/* Sets the keep focus state flag 
 * NOTE: Client must already be in focus to work.
 */
void setkeepfocus(Client *c, uint8_t state);
/* Sets the "Maximized Vert" Flag */
void setmaximizedvert(Client *c, uint8_t state);
/* Sets the "Maximized Horz" Flag */
void setmaximizedhorz(Client *c, uint8_t state);
/* Sets the Clients IS Shaded Flag. */
void setshaded(Client *c, uint8_t state);
/* Sets the Clients IS Modal Flag. */
void setmodal(Client *c, uint8_t state);
/* Sets override redirect flag, which disallows attaching to any linked list for a desktop 
 * But still allows a client to be found using wintoclient()
 */
void setoverrideredirect(Client *c, uint8_t state);
/* Replaces the Clients state with the sticky state, and sets IS sticky Flag. */
void setsticky(Client *c, uint8_t state);
/* Updates a Clients state to Urgent, and sets the Urgent Flag. (Updates window border to urgen color.) */
void seturgent(Client *c, uint8_t isurgent);
/* Shows client if ISVISIBLE(c);
 * else hides client.
 */
void showhide(Client *c);
/* Unfocuses specified client and sets to focus to root if setfocus is true */
void unfocus(Client *c, uint8_t setfocus);
/* updates a clients classname from XCBWMClass *_class 
 * No side effects on non filled _class dataw;
 */
void updateclass(Client *c, XCBWMClass *_class);
void updatedecor(Client *c);
/* Updates the Client icon if we find one */
void updateicon(Client *c, XCBWindowProperty *iconprop);
/* Updates the Clients border properties to the XServer */
void updateborder(Client *c);
/* Updates the Clients border colour to the XServer */
void updatebordercol(Client *c);
/* Updates the Clients border width to the XServer */
void updateborderwidth(Client *c);
/* updates motif hints if they are set */
void updatemotifhints(Client *c, XCBWindowProperty *motifprop);
/* Updates a Clients sizehints property using the provided hints pointer "size".
 * Doesnt require any data from client, AKA modular. still requires "size" though.
 */
void updatesizehints(Client *c, XCBSizeHints *size);
/* Updates Client tile if we find one;
 * if none found default to dwm.h BROKEN
 */
void updatetitle(Client *c, char *netwmname, char *wmname);
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
/* Returns the client if found from the specified window 
 * RETURN: Client * on Success.
 * RETURN: NULL on Failure.
 */
Client *wintoclient(XCBWindow win);
/* Unmanages Client AKA we dont tell it what todo Nor does it use our resources;
 * And perform checks based on specified "destroyed";
 * 1 -> widthdraw window;
 * 0 -> skip checks (window already destroyed)
 */
void unmanage(Client *c, uint8_t destroyed);
/* unmaximizes a client if maxed, Sets flag. */
void unmaximize(Client *c);
/* unmaximizes a client horizontally if maxed horz, Sets flag. */
void unmaximizehorz(Client *c);
/* unmaximizes a client vertically if maxed vert, Sets flag. */
void unmaximizevert(Client *c);

/* MACROS */

int ISALWAYSONTOP(Client *c);
int ISALWAYSONBOTTOM(Client *c);
int WASFLOATING(Client *c);
int ISFLOATING(Client *c);
int ISOVERRIDEREDIRECT(Client *c);
int KEEPFOCUS(Client *c);
int DISABLEBORDER(Client *c);
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
int WTYPENONE(Client *c);

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
int WSTATENONE(Client *c);

/* WM Protocol */
int HASWMTAKEFOCUS(Client *c);
int HASWMSAVEYOURSELF(Client *c);
int HASWMDELETEWINDOW(Client *c);

uint16_t OLDWIDTH(Client *c);
uint16_t OLDHEIGHT(Client *c);
uint16_t WIDTH(Client *c);
uint16_t HEIGHT(Client *c);









#endif
