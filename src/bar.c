#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bar.h"
#include "monitor.h"
#include "desktop.h"
#include "util.h"
#include "client.h"
#include "settings.h"

extern UserSettings _cfg;

/* Helper functions */
static uint32_t 
__intersect_area(
        /* rect 1 */
        int32_t x1, 
        int32_t y1, 
        int32_t x2, 
        int32_t y2,
        /* rect 2 */
        int32_t x3,
        int32_t y3,
        int32_t x4,
        int32_t y4
        )
{
    const int32_t xoverlap = MAX(0, MIN(x2, x4) - MAX(x1, x3));
    const int32_t yoverlap = MAX(0, MIN(y2, y4) - MAX(y1, y3));
    const uint32_t area = xoverlap * yoverlap;
    return area;
}

enum BarSides NOINLINE
calculatebarside(
        Monitor *m,
        Client *bar,
        uint8_t get_prev
        )
{
    const float LEEWAY = .15f;
    const float LEEWAY_SIDE = .35f;


    enum
    {   
        __BAR__MIN__X__,
        __BAR__MIN__Y__,
        __BAR__MAX__X__,
        __BAR__MAX__Y__
    };

    enum
    {   
        MIN_MAX_LENGTH = 4,
    };
    const i32 MODIFIERS[BarSideLAST][MIN_MAX_LENGTH] = 
    {
        [BarSideLeft] = 
        {
            [__BAR__MIN__X__] = 0,
            [__BAR__MIN__Y__] = (m->mh * LEEWAY),
            [__BAR__MAX__X__] = (m->mw * LEEWAY_SIDE),
            [__BAR__MAX__Y__] = (m->mh - LEEWAY)
        },
        [BarSideRight] = 
        {
            [__BAR__MIN__X__] = (m->mw - (m->mw * LEEWAY_SIDE)),
            [__BAR__MIN__Y__] = (m->mh * LEEWAY),
            [__BAR__MAX__X__] = (m->mw),
            [__BAR__MAX__Y__] = (m->mh - (m->mh * LEEWAY))
        },
        [BarSideTop] = 
        {            
            [__BAR__MIN__X__] = 0,
            [__BAR__MIN__Y__] = 0,
            [__BAR__MAX__X__] = m->mw,
            [__BAR__MAX__Y__] = (m->mh * LEEWAY)
        },
        [BarSideBottom] = 
        {            
           [__BAR__MIN__X__] = 0,
           [__BAR__MIN__Y__] = (m->mh - (m->mh * LEEWAY)),
           [__BAR__MAX__X__] = m->mw,
           [__BAR__MAX__Y__] = m->mh
        }
    };

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

    u32 bar_areas[BarSideLAST];
    u8 i;
    for(i = 0; i < BarSideLAST; ++i)
    {
        bar_areas[i] = __intersect_area(
                    m->mx + MODIFIERS[i][__BAR__MIN__X__],
                    m->my + MODIFIERS[i][__BAR__MIN__Y__],
                    m->mw + MODIFIERS[i][__BAR__MAX__X__],
                    m->mh + MODIFIERS[i][__BAR__MAX__Y__],
                    bx1,
                    by1,
                    bx2,
                    by2
                    );
    }
    /* the enum side doesnt matter but BarSideLeft is just 0, so start with that. */
    enum BarSides side = BarSideLeft;
    for(i = 0; i < BarSideLAST; ++i)
    { 
        /* gets the biggest area, while mataining correct side to return. */
        if(bar_areas[i] > bar_areas[side])
        {   side = i;
        }
    }
    return side;
}

u8
checknewbar(
        Monitor *m,
        Client *c,
        u8 has_strut_or_strutp
        )
{
    if(!m || !c)
    {   return false;
    }
    /* is there already a bar? */
    if(m->bar)
    {   return false;
    }
    const u8 sticky = !!ISSTICKY(c);
    const u8 isdock = !!ISDOCK(c);
    const u8 above = !!ISABOVE(c); 
    return (sticky && has_strut_or_strutp && (above || isdock));
}

void
setupbar(
        Monitor *m,
        Client *bar
        )
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

    f32 bxr;
    f32 byr;
    f32 bwr;
    f32 bhr;
    enum BarSides side = calculatebarside(m, bar, 0);
    enum BarSides prev = calculatebarside(m, bar, 1);
    if(prev != side)
    {
        switch(side)
        {   
            case BarSideLeft:
                bxr = _cfg.lx;
                byr = _cfg.ly;
                bwr = _cfg.lw;
                bhr = _cfg.lh;
                break;
            case BarSideRight:
                bxr = _cfg.rx;
                byr = _cfg.ry;
                bwr = _cfg.rw;
                bhr = _cfg.rh;
                break;
            case BarSideTop:
                bxr = _cfg.tx;
                byr = _cfg.ty;
                bwr = _cfg.tw;
                bhr = _cfg.th;
                break;
            /* if we fuck up default to bottom bar */
            case BarSideLAST:
                Debug0("BAR STATUS IS WRONG, FIXME");
            case BarSideBottom:
                bxr = _cfg.bx;
                byr = _cfg.by;
                bwr = _cfg.bw;
                bhr = _cfg.bh;
                break;
        }
        resize(bar, m->mx + (m->mw * bxr), m->my + (m->mh * byr), m->mw * bwr, m->mh * bhr, 1);
    }
    else
    {
        switch(side)
        {
            case BarSideLeft:
                _cfg.lx = bar->x;
                _cfg.ly = bar->y;
                _cfg.lw = bar->w;
                _cfg.lh = bar->h;
                break;
            case BarSideRight:
                _cfg.rx = bar->x;
                _cfg.ry = bar->y;
                _cfg.rw = bar->w;
                _cfg.rh = bar->h;
                break;
            case BarSideTop:
                _cfg.tx = bar->x;
                _cfg.ty = bar->y;
                _cfg.tw = bar->w;
                _cfg.th = bar->h;
                break;
            /* if we fuck up default to bottom bar */
            case BarSideLAST:
                Debug0("BAR STATUS IS WRONG, FIXME");
            case BarSideBottom:
                _cfg.bx = bar->x;
                _cfg.by = bar->y;
                _cfg.bw = bar->w;
                _cfg.bh = bar->h;
                break;
        }
    }
}

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
    enum BarSides side = calculatebarside(m, bar, 0);
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
    if(!ISHIDDEN(bar))
    {
        switch(side)
        {
            case BarSideLeft:
                /* make sure its on the left side */
                resize(bar, m->mx, m->my, bar->w, bar->h, 1);
                m->wx += bar->w;
                m->ww -= bar->w;
                Debug0("Bar Placed Left.");
                break;
            case BarSideRight:
                /* make sure its on the right side */
                resize(bar, m->mx + (m->mw - bar->w), m->my, bar->w, bar->h, 1);
                m->ww -= bar->w;
                Debug0("Bar Placed Right.");
                break;
            case BarSideTop:
                /* make sure its on the top side */
                resize(bar, m->mx, m->my, bar->w, bar->h, 1);
                m->wy += bar->h;
                m->wh -= bar->h;
                Debug0("Bar Placed Top.");
                break;
            case BarSideBottom:
                /* make sure its on the bottom side */
                resize(bar, m->mx, m->my + (m->mh - bar->h), bar->w, bar->h, 1);
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
                bar->x = -bar->w;
                break;
            case BarSideRight:
                bar->x = m->mw + bar->w;
                break;
            case BarSideTop:
                bar->y = -bar->h;
                break;
            case BarSideBottom:
                bar->y = m->mh + bar->h;
                break;
            default:
                /* just warp offscreen */
                bar->x = m->mw;
                bar->y = m->mh;
                break;
        }
    }
    resize(bar, bar->x, bar->y, bar->w, bar->h, 1);
}

