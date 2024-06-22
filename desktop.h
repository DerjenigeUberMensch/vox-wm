#ifndef _WM_DESKTOP_H
#define _WM_DESKTOP_H

#include "client.h"
/* layout(s) */
enum LayoutType
{
    Tiled, Floating, Monocle, Grid, LayoutTypeLAST
};

typedef struct Desktop Desktop;
typedef struct Layout Layout;
typedef struct UserSettings UserSettings;

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

struct Layout
{   
    void (*arrange)(Desktop *);
};


/* quickly calculate arrange stuff */
void arrangeq(Desktop *desk);
/* Arranges and restacks the windows in the specified desktop.
*/
void arrange(Desktop *desk);
/* Arranges the windows in the specified desktop.
 *
 * NOTE: Does not restack windows.
 */
void arrangedesktop(Desktop *desk);
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
/* Adds Client to focus linked list after specified "start" Client */
void attachfocusafter(Client *start, Client *after);
/* Adds Client to focus linked list before specified "start" Client */
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
/* Frees desktop and allocated desktop properties.
*/
void cleanupdesktop(Desktop *desk);
/* Allocates a desktop and desktop properties with all data set to 0 or to the adress of any newly allocated data.
 * RETURN: Desktop * on Success.
 * RETURN: NULL on Failure.
 */
Desktop *createdesktop(void);
/* Sets the "floating" layout for the specified desktop.
 * Floating -> Windows overlap each other AKA default win10 window behaviour.
 */
void floating(Desktop *desk);
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
/* Returns the previous desktop avaible. 
 * RETURN: Desktop * on Success.
 * RETURN: NULL on Failure.
 */
Desktop *prevdesktop(Desktop *desk);
/* Reorders(restacks) clients in current desk->stack */
void restack(Desktop *desk);
/* "Restacks" clients on from linked list no effect unless restack called*/
void reorder(Desktop *desk);
/* Sets the desktops layouts, (not automatic arrange must be called after to apply changes.) */
void setdesktoplayout(Desktop *desk, uint8_t layout);
/*      reference point is c1.
 *      so if c1 has higher priority return 1.
 *      RETURN: 1 on higher priority.
 *      RETURN: 0 on lesser priority.
 */
int stackpriority(Client *c1, Client *c2);
/* Sets the "Tiled" layout for the specified desktop.
 * Tiled -> Windows tile in a grid like patter where there is 1 Big window to the left,
 *          and "stacking" on top of each other smaller windows on the right.
 */
void tile(Desktop *desk);
/* Updates the XServer to the Current destop */
void updatedesktop(void);
/* Updates the desktop names if they have changed */
void updatedesktopnames(void);
/* Updates the current desktop count AKA how many desktops we got to the XServer */
void updatedesktopnum(void);
/* Updates focus order when before reorder() */
void updatestackpriorityfocus(Desktop *desk);
/* updates the viewport property to the XServer */
void updateviewport(void);


static const Layout layouts[LayoutTypeLAST] =
{
    /* Name             arrange     */
    [Tiled]     = {     tile        },
    [Floating]  = {     floating    },
    [Monocle]   = {     monocle     },
    [Grid]      = {     grid        },
};

#endif
