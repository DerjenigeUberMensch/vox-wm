
#include "client.h"
#include "monitor.h"
#include "desktop.h"
#include "dwm.h"



extern WM _wm;
extern UserSettings _cfg;
extern XCBAtom netatom[];


void 
arrangeq(Desktop *desk)
{
    reorder(desk);
    restack(desk);
}

void
arrange(Desktop *desk)
{
    /* bar stuff */
    updatebargeom(desk->mon);
    updatebarpos(desk->mon);

    reorder(desk);
    arrangedesktop(desk);
    restack(desk);
}

void
arrangedesktop(Desktop *desk)
{   
    if(layouts[desk->layout].arrange)
    {   layouts[desk->layout].arrange(desk);
    }
}


/* Macro helper */
#define __attach_helper(STRUCT, HEAD, NEXT, PREV, LAST)     do                                      \
                                                            {                                       \
                                                                STRUCT->NEXT = STRUCT->HEAD;        \
                                                                STRUCT->HEAD = STRUCT;              \
                                                                if(STRUCT->NEXT)                    \
                                                                {   STRUCT->NEXT->PREV = STRUCT;    \
                                                                }                                   \
                                                                else                                \
                                                                {   STRUCT->LAST = STRUCT;          \
                                                                }                                   \
                                                                /* prevent circular linked list */  \
                                                                STRUCT->PREV = NULL;                \
                                                            } while(0)

#define __attach_after(START, AFTER, NEXT, PREV, HEAD, LAST)        do                                  \
                                                                    {                                   \
                                                                        AFTER->NEXT = START->NEXT;      \
                                                                        AFTER->PREV = START;            \
                                                                        if(!START->NEXT)                \
                                                                        {   LAST = START;               \
                                                                        }                               \
                                                                        else                            \
                                                                        {   START->NEXT->PREV = AFTER;  \
                                                                        }                               \
                                                                        START->NEXT = AFTER;            \
                                                                    } while(0)
#define __attach_before(START, BEFORE, NEXT, PREV, HEAD, LAST, ATTACH)  do                                  \
                                                                        {                                   \
                                                                            if(!START->PREV)                \
                                                                            {   ATTACH(BEFORE);             \
                                                                            }                               \
                                                                            else                            \
                                                                            {                               \
                                                                                BEFORE->PREV = START->PREV; \
                                                                                BEFORE->NEXT = START;       \
                                                                                START->PREV->NEXT = BEFORE; \
                                                                                START->PREV = BEFORE;       \
                                                                            }                               \
                                                                        } while(0)

/* Too hard to implement */
/*
#define __detach_helper(TYPE, STRUCT, HEAD, NEXT, PREV, LAST)   do                                                              \
                                                                {                                                               \
                                                                    TYPE **tc;                                                  \
                                                                    for(tc = &STRUCT->HEAD; *tc && *tc != STRUCT; tc = &(*tc)->NEXT);   \
                                                                    *tc = STRUCT->NEXT;                                         \
                                                                    if(!(*tc))                                                  \
                                                                    {   STRUCT->LAST = STRUCT->PREV;                            \
                                                                    }                                                           \
                                                                    else if(STRUCT->NEXT)                                       \
                                                                    {   STRUCT->NEXT->PREV = STRUCT->PREV;                      \
                                                                    }                                                           \
                                                                    else if(STRUCT->PREV)                                       \
                                                                    {                                                           \
                                                                        STRUCT->LAST = STRUCT->PREV;                            \
                                                                        STRUCT->PREV->NEXT = NULL;                              \
                                                                    }                                                           \
                                                                    STRUCT->NEXT = NULL;                                        \
                                                                    STRUCT->PREV = NULL;                                        \
                                                                } while(0)
*/

void
attach(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->clients, next, prev, desktop->clast);
}

void
attachstack(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->stack, snext, sprev, desktop->slast);
}

void
attachrestack(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->rstack, rnext, rprev, desktop->rlast);
}

void
attachfocus(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->focus, fnext, fprev, desktop->flast);
}

void
attachfocusafter(Client *start, Client *after)
{
    if(ISOVERRIDEREDIRECT(after))
    {   return;
    }
    Desktop *desk = start->desktop;
    detachfocus(after);
    __attach_after(start, after, fnext, fprev, desk->focus, desk->slast);
}

void
attachfocusbefore(Client *start, Client *after)
{
    if(ISOVERRIDEREDIRECT(after))
    {   return;
    }
    Desktop *desk = start->desktop;
    detachfocus(after);
    __attach_before(start, after, fnext, fprev, desk->focus, desk->slast, attachfocus);
}

void
detach(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->clients)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Client **tc;
    for (tc = &c->desktop->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
    if(!(*tc)) 
    {
        c->desktop->clast = c->prev;
    }
    else if(c->next) 
    {   c->next->prev = c->prev;
    }
    else if(c->prev) 
    {   /* This should be UNREACHABLE but in case we do reach it then this should suffice*/
        c->desktop->clast = c->prev;
        c->prev->next = NULL;
    }
    c->next = NULL;
    c->prev = NULL;
}

void
detachcompletely(Client *c)
{
    detach(c);
    detachstack(c);
    detachfocus(c);
    detachrestack(c);
}

void
detachstack(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->stack)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc;

    for(tc = &desk->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;
    if(!(*tc))
    {
        desk->slast = c->sprev;
    }
    else if(c->snext)
    {   
        c->snext->sprev = c->sprev;
    }
    else if(c->sprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->slast = c->sprev;
        c->sprev->snext = NULL;
    }
    c->sprev = NULL;
    c->snext = NULL;
}

void
detachrestack(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->rstack)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc;

    for(tc = &desk->rstack; *tc && *tc != c; tc = &(*tc)->rnext);
    *tc = c->rnext;
    if(!(*tc))
    {
        desk->rlast = c->rprev;
    }
    else if(c->rnext)
    {   
        c->rnext->rprev = c->rprev;
    }
    else if(c->rprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->rlast = c->rprev;
        c->rprev->rnext = NULL;
    }

    c->rprev = NULL;
    c->rnext = NULL;
}

void
detachfocus(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->focus)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc, *t;

    for(tc = &desk->focus; *tc && *tc != c; tc = &(*tc)->fnext);
    *tc = c->fnext;
    if(!(*tc))
    {
        desk->flast = c->fprev;
    }
    else if(c->fnext)
    {   
        c->fnext->fprev = c->fprev;
    }
    else if(c->fprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->flast = c->fprev;
        c->fprev->fnext = NULL;
    }

    /* this just updates desktop->sel */
    if (c == c->desktop->sel)
    {
        for (t = c->desktop->focus; t && !ISVISIBLE(t); t = t->fnext);
        c->desktop->sel = t;
    }

    c->fprev = NULL;
    c->fnext = NULL;
}

void
cleanupdesktop(Desktop *desk)
{
    Client *c = NULL;
    Client *next = NULL;
    c = desk->clients;
    while(c)
    {   
        next = c->next;
        cleanupclient(c);
        c = next;
    }
    free(desk->settings);
    free(desk);
    desk = NULL;
}

Desktop *
createdesktop(void)
{
    Desktop *desk = calloc(1, sizeof(Desktop));
    if(!desk)
    {
        DEBUG("%s", "WARN: FAILED TO CREATE DESKTOP");
        return NULL;
    }
    desk->layout = 0;
    desk->olayout= 0;
    desk->clients= NULL;
    desk->clast = NULL;
    desk->stack = NULL;
    desk->slast = NULL;
    desk->mon = NULL;
    desk->settings = calloc(1, sizeof(UserSettings));
    if(!desk->settings)
    {   
        DEBUG("%s", "WARN: FAILED TO CREATE SETTINGS.");
        free(desk);
        return NULL;
    }
    return desk;
}

void
floating(Desktop *desk)
{
    /* for now just check in restack for it */
    monocle(desk);
}

void
grid(Desktop *desk)
{
    const float bgwr = USGetGapRatio(&_cfg);

    i32 n, cols, rows, cn, rn, i, cx, cy, cw, ch;
    i32 nx, ny;
    i32 nw, nh;
	Client *c;
    Monitor *m = desk->mon;

	for(n = 0, c = nexttiled(desk->stack); c; c = nexttiled(c->snext), n++);
	if(n == 0)
    {   return;
    }

	/* grid dimensions */
	for(cols = 0; cols <= n/2; cols++)
    {   
        if(cols*cols >= n)
        {    break;
        }
    }
    /* set layout against the general calculation: not 1:2:2, but 2:3 */
	if(n == 5) 
    {   cols = 2;
    }
	rows = n / cols;

	/* window geometries */
    cw = m->ww / (cols + !cols);
    /* current column number */
	cn = 0;
    /* current row number */
	rn = 0; 
	for(i = 0, c = nexttiled(desk->stack); c; i++, c = nexttiled(c->snext)) 
    {
		if((i / rows) + 1 > cols - (n % cols))
        {   rows = n/cols + 1;
        }
        ch = m->wh / (rows + !rows);
		cx = m->wx + cn * cw;
		cy = m->wy + rn * ch;

        nx = cx;
        ny = cy;
        nw = cw - (WIDTH(c) - c->w);
        nh = ch - (HEIGHT(c) - c->h);

        nx += (nw - (nw * bgwr)) / 2;
        ny += (nh - (nh * bgwr)) / 2;
        nw *= bgwr;
        nh *= bgwr;

		resize(c, nx, ny, nw, nh, 0);
        ++rn;
		if(rn >= rows) 
        {
			rn = 0;
            ++cn;
		}
	}
}

void
monocle(Desktop *desk)
{
    Client *c;
    Monitor *m = desk->mon;
    i32 nw, nh;
    const i32 nx = m->wx;
    const i32 ny = m->wy;

    for(c = nexttiled(desk->stack); c; c = nexttiled(c->snext))
    {
        nw = m->ww - (c->bw * 2);
        nh = m->wh - (c->bw * 2);
        resize(c, nx, ny, nw, nh, 0);
    }
}

Desktop *
nextdesktop(Desktop *desk)
{
    return desk ? desk->next : desk;
}

Monitor *
nextmonitor(Monitor *m)
{
    return m ? m->next : m;
}

Desktop *
prevdesktop(Desktop *desk)
{
    return desk ? desk->prev : desk;
}

void
restack(Desktop *desk)
{
    XCBWindowChanges wc;

    wc.stack_mode = XCB_STACK_MODE_BELOW;

    if(desk->mon->bar && !ISHIDDEN(desk->mon->bar))
    {   wc.sibling = desk->mon->bar->win;
    }
    else
    {   /* TODO: Maybe use wc.sibling = _wm.root? That causes error to be generated though. */
        wc.sibling = _wm.wmcheckwin;
    }

    Client *c = NULL;
    u8 config = 0;
    u8 instack = 0;
    
    c = desk->stack;
    /* reset client list */
    if(c)
    {   XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientListStacking], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&c->win, 1);
    }
    else
    {   XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetClientListStacking]);
    }
    for(c = desk->stack; c; c = nextstack(c))
    {
        instack = c->rprev || c->rnext;
        /* Client holds both lists so we just check if the next's are the same if not configure it */
        config = c->rnext != c->snext || !instack;
        if(config)
        {   
            XCBConfigureWindow(_wm.dpy, c->win, XCB_CONFIG_WINDOW_SIBLING|XCB_CONFIG_WINDOW_STACK_MODE, &wc);
            DEBUG("Configured window: %s", c->netwmname);
        }
        /* add clients to Stack list (some apps use this) */
        if(nextstack(c))
        {   
            XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientListStacking], XCB_ATOM_WINDOW, 32, 
                    XCB_PROP_MODE_PREPEND, (unsigned char *)&(nextstack(c))->win, 1);
        }
        wc.sibling = c->win;
        c->rprev = c->sprev;
        c->rnext = c->snext;
    }
    desk->rstack = desk->stack;
    desk->rlast = desk->slast;
}

void
reorder(Desktop *desk)
{
    updatestackpriorityfocus(desk);
    MERGE_SORT_LINKED_LIST(Client, stackpriority, desk->stack, desk->slast, snext, sprev, 1, 0);
}

/* how to return.
 * reference point is c1.
 *
 * sort order is 1,2,3,4,5,6
 */
static inline int
__stack_priority_helper_above(const unsigned int x1, const unsigned int x2)
{
    return x1 < x2;
}
/* how to return.
 * reference point is c1.
 *
 * sort order is 6,5,4,3,2,1
 */
static inline int 
__stack_priority_helper_below(const unsigned int x1, const unsigned int x2)
{
    return x1 > x2;
}

void
setdesktoplayout(Desktop *desk, uint8_t layout)
{
    desk->olayout = desk->layout;
    desk->layout = layout;
}

int
stackpriority(Client *c1, Client *c2)
{
    const unsigned int dock1 = ISDOCK(c1);
    const unsigned int dock2 = ISDOCK(c2);

    const unsigned int above1 = ISALWAYSONTOP(c1);
    const unsigned int above2 = ISALWAYSONTOP(c2);

    const unsigned int float1 = ISFAKEFLOATING(c1);
    const unsigned int float2 = ISFAKEFLOATING(c2);

    const unsigned int below1 = ISBELOW(c1);
    const unsigned int below2 = ISBELOW(c2);

    const unsigned int hidden1 = ISHIDDEN(c1);
    const unsigned int hidden2 = ISHIDDEN(c2);

    /* Bottom Restacking */
    if(below1 ^ below2)
    {   
        return __stack_priority_helper_below(below1, below2);
    }
    else if(hidden1 ^ hidden2)
    {   
        return __stack_priority_helper_below(hidden1, hidden2);
    }
    /* Regular restacking */
    else if(dock1 ^ dock2)
    {   
        return __stack_priority_helper_above(dock1, dock2);
    }
    else if(above1 ^ above2)
    {   
        return __stack_priority_helper_above(above1, above2);
    }
    else if(float1 ^ float2)
    {   
        return __stack_priority_helper_above(float1, float2);
    }
    /* focus is forward order so, we must calculate reversely */
    return __stack_priority_helper_above(c2->rstacknum, c1->rstacknum);
}

void
tile(Desktop *desk)
{
    const u16 nmaster = USGetMCount(&_cfg);
    const float mfact = USGetMFact(&_cfg);
    const float bgwr = USGetGapRatio(&_cfg);

    i32 h, mw, my, ty;
    i32 n, i;
    i32 nx, ny;
    i32 nw, nh;

    Client *c = NULL;
    Monitor *m = desk->mon;

    n = 0;
    for(c = nexttiled(desk->stack); c; c = nexttiled(c->snext), ++n);

    if(!n) 
    {   return;
    }
    
    if(n > nmaster)
    {   mw = nmaster ? m->ww * mfact: 0;
    }
    else
    {   mw = m->ww;
    }

    i = my = ty = 0;
    for (c = nexttiled(desk->stack); c; c = nexttiled(c->snext))
    {
        if (i < nmaster)
        {
            h = (m->wh - my) / (MIN(n, nmaster) - i);
            nx = m->wx;
            ny = m->wy + my;
            nw = mw - c->bw * 2;
            nh = h - c->bw * 2;

            nx += (nw - (nw * bgwr)) / 2;
            ny += (nh - (nh * bgwr)) / 2;
            nw *= bgwr;
            nh *= bgwr;

            resize(c, nx, ny, nw, nh, 0);
            if (my + HEIGHT(c) < m->wh) 
            {   my += HEIGHT(c);
            }
        }
        else
        {
            h = (m->wh - ty) / (n - i);
            nx = m->wx + mw;
            ny = m->wy + ty;
            nw = m->ww - mw - c->bw * 2;
            nh = h - c->bw * 2;

            nx += (nw - (nw * bgwr)) / 2;
            ny += (nh - (nh * bgwr)) / 2;
            nw *= bgwr;
            nh *= bgwr;

            resize(c, nx, ny, nw, nh, 0);
            if (ty + HEIGHT(c) < m->wh) 
            {   ty += HEIGHT(c);
            }
        }
        ++i;
    }
}

void
updatedesktop(void)
{
    u32 data = _wm.selmon->desksel->num;
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&data, 1);
}

void
updatedesktopnames(void)
{
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopNames], XCB_ATOM_STRING, 8, XCB_PROP_MODE_REPLACE, "~0", 3);
}

void
updatedesktopnum(void)
{
    i32 data =  _wm.selmon->deskcount;
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetNumberOfDesktops], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&data, 1);
}

void
updatestackpriorityfocus(Desktop *desk)
{
    Client *c;
    int i = 0;
    for(c = desk->focus; c; c = nextfocus(c))
    {
        c->rstacknum = ++i;
        if(ISFLOATING(c) && DOCKED(c))
        {   setfloating(c, 0);
        }
    }
}

void
updateviewport(void)
{
    i32 data[2] = { 0, 0 };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopViewport], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}


