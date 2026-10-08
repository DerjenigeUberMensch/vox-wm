#include <stdio.h>
#include <stdlib.h>

#include "bar.h"
#include "XCB-TRL/xcb_winutil.h"
#include "client.h"
#include "settings.h"
#include "main.h"
#include "util.h"
#include "x.h"

extern WM _wm;
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern UserSettings _cfg;

u32 ISBAR(Client *c) 
                                {
                                    return (ISDOCK(c) && HASSTRUT(c)) || (ISABOVE(c) && ISDOCK(c) && ISSTICKY(c));
                                }


enum BarSides GETBARSIDE(Monitor *m, Client *bar, uint8_t get_prev)
                                {
                                    /* ask chatpgt ikd */
                                    f64 x = get_prev ? bar->oldx : bar->x;
                                    f64 y = get_prev ? bar->oldy : bar->y;
                                    f64 w = get_prev ? bar->oldw : bar->w;
                                    f64 h = get_prev ? bar->oldh : bar->h;

                                    if(unlikely(m->mw <= 0 || m->mh <= 0))
                                    {   
                                        DebugError("Invalid monitor geometry: %dx%d", m->mw, m->mh);
                                        return BarSideTop;
                                    }

                                    /* 
                                     * -1 -> left/top edge
                                     *  0 -> center
                                     *  1 -> right/bottom edge
                                     */

                                    f64 cx = 2.0 * ( (x - m->mx + w * .5) / m->mw) - 1.0;
                                    f64 cy = 2.0 * ( (y - m->my + h * .5) / m->mh) - 1.0;

                                    if(ISFIXED(bar))
                                    {
                                        if(w > h)
                                        {   return cy <= 0.0 ? BarSideTop : BarSideBottom;
                                        }

                                        if(h > w)
                                        {   return cx <= 0.0 ? BarSideLeft : BarSideRight;
                                        }

                                        DebugOnce("Bar is a square, suprisingly...");
                                    }

                                    f64 ax = cx < 0.0 ? -cx : cx;
                                    f64 ay = cy < 0.0 ? -cy : cy;

                                    if(ax > ay)
                                    {   return cx < 0.0 ? BarSideLeft : BarSideRight;
                                    }

                                    return cy < 0.0 ? BarSideTop : BarSideBottom;
                                }

void
setupbar(Monitor *m, Client *bar)
{
    /* some implementations dont handle X11 atoms properly (cough cough alttab) so this is a workaround */
    if(ISDOCK(bar))
    {
        if(!SKIPTASKBAR(bar))
        {   XCBChangeProperty(_wm.dpy, bar->win, netatom[NetWMState], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, &netatom[NetWMStateSkipTaskbar], 1);
        }

        setskiptaskbar(bar, 1);
    }

    setborderwidth(bar, 0);
    updateborder(bar);
    setdisableborder(bar, 1);
    setsticky(bar, 1);
}

void
updatebargeom(Desktop *desk)
{
    Monitor *m = desk->mon;

    Client *c;
    Generic bxr;
    Generic byr;
    Generic bwr;
    Generic bhr;
    enum BarSides side;
    enum BarSides prev;

    for(c = startstack(desk); c; c = nextstack(c))
    {
        if(!ISBAR(c))
        {   continue;
        }

        if(ISHIDDEN(c))
        {   continue;
        }

        /* if the bar is fixed then the geom is impossible to update, also we dont want to update our current bar status cause of that also */
        if(ISFIXED(c))
        {   continue;
        }

        side = GETBARSIDE(m, c, 0);
        prev = GETBARSIDE(m, c, 1);

        if(prev != side)
        {
            i32 x;
            i32 y;
            i32 w;
            i32 h;
            switch(side)
            {   
                case BarSideLeft:
                    bxr = USGetSetting(&_cfg, BarLX);
                    byr = USGetSetting(&_cfg, BarLY);
                    bwr = USGetSetting(&_cfg, BarLW);
                    bhr = USGetSetting(&_cfg, BarLH);
                    break;
                case BarSideRight:
                    bxr = USGetSetting(&_cfg, BarRX);
                    byr = USGetSetting(&_cfg, BarRY);
                    bwr = USGetSetting(&_cfg, BarRW);
                    bhr = USGetSetting(&_cfg, BarRH);
                    break;
                case BarSideTop:
                    bxr = USGetSetting(&_cfg, BarTX);
                    byr = USGetSetting(&_cfg, BarTY);
                    bwr = USGetSetting(&_cfg, BarTW);
                    bhr = USGetSetting(&_cfg, BarTH);
                    break;
                case BarSideBottom:
                    bxr = USGetSetting(&_cfg, BarBX);
                    byr = USGetSetting(&_cfg, BarBY);
                    bwr = USGetSetting(&_cfg, BarBW);
                    bhr = USGetSetting(&_cfg, BarBH);
                    break;
            }

            x = m->mx + (m->mw * bxr.dataf[0]);
            y = m->my + (m->mh * byr.dataf[0]);
            w = m->mw * bwr.dataf[0];
            h = m->mh * bhr.dataf[0];

            resize(c, x, y, w, h, 1);
        }
        else
        {
            f32 x = c->x;
            f32 y = c->y;
            f32 w = c->w;
            f32 h = c->h;

            f32 mw = m->mw;
            f32 mh = m->mh;

            if(!ASSERT(mw != 0 && mh != 0))
            {   continue;
            }

            /* prevent div by 0 hardware exceptions */
            bxr = (Generic) { .dataf[0] = (x - m->mx) / m->mw };
            byr = (Generic) { .dataf[0] = (y - m->my) / m->mh };
            bwr = (Generic) { .dataf[0] = w / m->mw };
            bhr = (Generic) { .dataf[0] = h / m->mh };

            switch(side)
            {
                case BarSideLeft:
                    USSetSetting(&_cfg, BarLX, bxr);
                    USSetSetting(&_cfg, BarLY, byr);
                    USSetSetting(&_cfg, BarLW, bwr);
                    USSetSetting(&_cfg, BarLH, bhr);
                    break;
                case BarSideRight:
                    USSetSetting(&_cfg, BarRX, bxr);
                    USSetSetting(&_cfg, BarRY, byr);
                    USSetSetting(&_cfg, BarRW, bwr);
                    USSetSetting(&_cfg, BarRH, bhr);
                    break;
                case BarSideTop:
                    USSetSetting(&_cfg, BarTX, bxr);
                    USSetSetting(&_cfg, BarTY, byr);
                    USSetSetting(&_cfg, BarTW, bwr);
                    USSetSetting(&_cfg, BarTH, bhr);
                    break;
                case BarSideBottom:
                    USSetSetting(&_cfg, BarBX, bxr);
                    USSetSetting(&_cfg, BarBY, byr);
                    USSetSetting(&_cfg, BarBW, bwr);
                    USSetSetting(&_cfg, BarBH, bhr);
                    break;
            }
        }
    }
}

void
updatebarpos(Desktop *desk)
{
    Monitor *m = desk->mon;
    Client *c;

    i32 loff = 0;
    i32 roff = 0;
    i32 toff = 0;
    i32 boff = 0;

    for(c = startstack(desk); c; c = nextstack(c))
    {
        if(!ISBAR(c))
        {   continue;
        }

        if(ISHIDDEN(c))
        {   continue;
        }

        enum BarSides side = GETBARSIDE(m, c, 0);

        i32 x = m->mx;
        i32 y = m->my;
        i32 w = c->w;
        i32 h = c->h;

        switch(side)
        {
            case BarSideLeft:
                x = m->mx + loff;
                y = m->my;

                loff += c->w;

                break;
            case BarSideRight:
                x = m->mx + m->mw - roff - c->w;
                y = m->my;

                roff += c->w;

                break;
            case BarSideTop:
                x = m->mx;
                y = m->my + toff;

                toff += c->h;

                break;
            case BarSideBottom:
                x = m->mx;
                y = m->my + m->mh - boff - c->h;

                boff += c->h;

                break;
            default: break;
        }

        resize(c, x, y, w, h, 1);
    }
}

void
updatebars(Desktop *desk)
{
    updatebargeom(desk);
    updateworkarea(desk);
    updatebarpos(desk);
}

void 
updateworkarea(Desktop *desk)
{
    Monitor *m = desk->mon;

    i32 left = 0;
    i32 right = 0;
    i32 top = 0;
    i32 bottom = 0;

    Client *c;

    for(c = startstack(desk); c; c = nextstack(c))
    {
        if(!ISBAR(c))
        {   continue;
        }

        if(ISHIDDEN(c))
        {   continue;
        }

        enum BarSides side = GETBARSIDE(m, c, 0);;

        if(ISFIXED(c))
        {
            if(c->w > c->h)
            {   
                /* is it top bar ? */
                if(c->y + c->h / 2 <= m->my + m->mh / 2)
                {   side = BarSideTop;
                }
                /* its bottom bar */
                else
                {   side = BarSideBottom;
                }
            }
            else if(c->w < c->h)
            {
                /* is it left bar? */
                if(c->x + c->w / 2 <= m->mx + m->mw / 2)
                {   side = BarSideLeft;
                }
                /* its right bar */
                else
                {   side = BarSideRight;
                }
            }
            else
            {   Debug0("Detected bar is a square suprisingly.");
            }
        }

        switch(side)
        {
            case BarSideLeft:   left   += c->w; break;
            case BarSideRight:  right  += c->w; break;
            case BarSideTop:    top    += c->h; break;
            case BarSideBottom: bottom += c->h; break;
            default: break;
        }
    }

    m->wx = m->mx + left;
    m->wy = m->my + top;

    m->ww = m->mw - left - right;
    m->wh = m->mh - top - bottom;

    if(unlikely(m->ww < 0))
    {   m->ww = 0;
    }

    if(unlikely(m->wh < 0))
    {   m->wh = 0;
    }
}
