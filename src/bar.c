#include <stdio.h>
#include <stdlib.h>


#include "bar.h"
#include "settings.h"
#include "main.h"

extern WM _wm;
extern UserSettings _cfg;

u32 COULDBEBAR(Client *c, uint8_t strut) 
                                {
                                    const u8 sticky = !!ISSTICKY(c);
                                    const u8 isdock = !!(ISDOCK(c));
                                    const u8 above = !!ISABOVE(c); 

                                    return (sticky && strut && above && isdock);
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

uint8_t
checknewbar(Monitor *m, Client *c, uint8_t has_strut_or_strutp)
{
    /* barwin checks */
    u8 checkbar = !m->bar;
    if(checkbar && COULDBEBAR(c, has_strut_or_strutp))
    {   
        setupbar(m, c);
        return 0;
    }
    return 1;
}

void
setupbar(Monitor *m, Client *bar)
{
    detachcompletely(bar);
    configure(bar);
    m->bar = bar;
    setoverrideredirect(bar, 1);
    setborderwidth(bar, 0);
    updateborder(bar);
    setdisableborder(bar, 1);
    setfullscreen(bar, 0);
    sethidden(bar, 0);
    setsticky(bar, 1);
    updatebargeom(m);
    updatebarpos(m);
    Debug("Found a bar: [%d]", bar->win);
}

void
updatebargeom(Monitor *m)
{
    Client *bar = m->bar;
    if(!bar || ISHIDDEN(bar))
    {   return;
    }
    /* if the bar is fixed then the geom is impossible to update, also we dont want to update our current bar status cause of that also */
    if(ISFIXED(bar))
    {   return;
    }

    Generic bxr;
    Generic byr;
    Generic bwr;
    Generic bhr;
    enum BarSides side = GETBARSIDE(m, bar, 0);
    enum BarSides prev = GETBARSIDE(m, bar, 1);

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

        resize(bar, x, y, w, h, 1);
    }
    else
    {
        f32 x = bar->x;
        f32 y = bar->y;
        f32 w = bar->w;
        f32 h = bar->h;

        f32 mw = m->mw;
        f32 mh = m->mh;

        if(!ASSERT(mw != 0 && mh != 0))
        {   return;
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

/*
void
USSetSetting(
        UserSettings *settings,
        unsigned int setting,
        Generic data
        )
*/

void
updatebarpos(Monitor *m)
{
    /* reset space */
    m->ww = m->mw;
    m->wh = m->mh;
    m->wx = m->mx;
    m->wy = m->my;
    Client *bar = m->bar;
    if(!bar)
    {   return;
    }
    enum BarSides side = GETBARSIDE(m, bar, 0);
    if(ISFIXED(bar))
    {
        if(bar->w > bar->h)
        {   
            /* is it top bar ? */
            if(bar->y + bar->h / 2 <= m->my + m->mh / 2)
            {   side = BarSideTop;
            }
            /* its bottom bar */
            else
            {   side = BarSideBottom;
            }
        }
        else if(bar->w < bar->h)
        {
            /* is it left bar? */
            if(bar->x + bar->w / 2 <= m->mx + m->mw / 2)
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

    i32 x;
    i32 y;
    i32 w;
    i32 h;

    /* default is top left side */
    x = m->mx;
    y = m->my;
    w = bar->w;
    h = bar->h;

    if(!ISHIDDEN(bar))
    {
        switch(side)
        {
            case BarSideLeft:
                m->wx += bar->w;
                m->ww -= bar->w;
                Debug0("Bar Placed Left.");
                break;
            case BarSideRight:
                x += m->mw - bar->w;
                m->ww -= bar->w;
                Debug0("Bar Placed Right.");
                break;
            case BarSideTop:
                m->wy += bar->h;
                m->wh -= bar->h;
                Debug0("Bar Placed Top.");
                break;
            case BarSideBottom:
                y += m->mh - bar->h;
                m->wh -= bar->h;
                Debug0("Bar Placed Bottom.");
                break;
            default:
                break;
        }
    }
    else
    {   
        switch(side)
        {
            case BarSideLeft:
                x = -bar->w;
                break;
            case BarSideRight:
                x = m->mw;
                break;
            case BarSideTop:
                y = -bar->h;
                break;
            case BarSideBottom:
                y = m->mh;
                break;
            default:
                /* just warp offscreen */
                x = m->mw;
                y = m->mh;
                break;
        }
    }
    resize(bar, x, y, w, h, 1);
}

