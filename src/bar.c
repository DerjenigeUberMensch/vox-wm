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
                                    return ISSTICKY(c) && ISDOCK(c) && ISABOVE(c) && HASSTRUT(c);
                                }


enum BarSides GETBARSIDE(Monitor *m, Client *bar, uint8_t get_prev)
                                { 
                                    const float LEEWAY = .15f;
                                    const float LEEWAY_SIDE = .35f;

                                    /* top parametors */
                                    const i32 TOP_MIN_X = m->mx;
                                    const i32 TOP_MIN_Y = m->my;

                                    const i32 TOP_MAX_X = m->mx + m->mw;
                                    const i32 TOP_MAX_Y = m->my + (m->mh * LEEWAY);

                                    /* bottom parametors */
                                    const i32 BOTTOM_MIN_X = m->mx;
                                    const i32 BOTTOM_MIN_Y = m->my + m->mh - (m->mh * LEEWAY);

                                    const i32 BOTTOM_MAX_X = m->mx + m->mw;
                                    const i32 BOTTOM_MAX_Y = m->my + m->mh;

                                    /* sidebar left parametors */
                                    const i32 LEFT_MIN_X = m->mx;
                                    const i32 LEFT_MIN_Y = TOP_MAX_Y;

                                    const i32 LEFT_MAX_X = m->mx + (m->mw * LEEWAY_SIDE);
                                    const i32 LEFT_MAX_Y = BOTTOM_MIN_Y;

                                    /* sidebar right parametors */
                                    const i32 RIGHT_MIN_X = m->mx + m->mw - (m->mw * LEEWAY_SIDE);
                                    const i32 RIGHT_MIN_Y = TOP_MAX_Y;

                                    const i32 RIGHT_MAX_X = m->mx + m->mw;
                                    const i32 RIGHT_MAX_Y = BOTTOM_MIN_Y;

                                    enum BarSides side;
                                    i32 bx1;
                                    i32 by1;
                                    i32 bx2;
                                    i32 by2;

                                    if(get_prev)
                                    {
                                        bx1 = bar->oldx + (bar->oldw / 2);
                                        by1 = bar->oldy + (bar->oldh / 2);
                                        bx2 = bx1 + bar->oldw;
                                        by2 = by1 + bar->oldh;
                                    }
                                    else
                                    {
                                        bx1 = bar->x + (bar->w / 2);
                                        by1 = bar->y + (bar->h / 2);
                                        bx2 = bx1 + bar->w;
                                        by2 = by1 + bar->h;
                                    }

                                    uint32_t toparea = __intersect_area(
                                            TOP_MIN_X,
                                            TOP_MIN_Y,
                                            TOP_MAX_X,
                                            TOP_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t bottomarea = __intersect_area(
                                            BOTTOM_MIN_X,
                                            BOTTOM_MIN_Y,
                                            BOTTOM_MAX_X,
                                            BOTTOM_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t leftarea = __intersect_area(
                                            LEFT_MIN_X,
                                            LEFT_MIN_Y,
                                            LEFT_MAX_X,
                                            LEFT_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t rightarea = __intersect_area(
                                            RIGHT_MIN_X,
                                            RIGHT_MIN_Y,
                                            RIGHT_MAX_X,
                                            RIGHT_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );

                                    uint32_t biggest = toparea;

                                    if(biggest < bottomarea)
                                    {   biggest = bottomarea;
                                    }

                                    if(biggest < leftarea)
                                    {   biggest = leftarea;
                                    }

                                    if(biggest < rightarea)
                                    {   biggest = rightarea;
                                    }

                                    /* prob should handle the rare change that the area would be the same as another,
                                     * But at that point we should just rework it to use buttonpress last pressed location.
                                     */
                                    if(biggest == toparea)
                                    {   side = BarSideTop;
                                    }
                                    else if(biggest == bottomarea)
                                    {   side = BarSideBottom;
                                    }
                                    else if(biggest == leftarea)
                                    {   side = BarSideLeft;
                                    }
                                    else if(biggest == rightarea)
                                    {   side = BarSideRight;
                                    }
                                    else    /* this is just for compiler ommiting warning */
                                    {   side = BarSideTop;
                                    }

                                    return side;
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
