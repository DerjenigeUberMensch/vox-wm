
#include <stdint.h>
#include <string.h>
/* POSIX strcasecmp()
 */
#include <strings.h>

#include "main.h"
#include "floating.h"
#include "client.h"
#include "monitor.h"
#include "desktop.h"
#include "usersettings.h"

extern WM _wm;
extern UserSettings _cfg;

int32_t MAX_ROUNDING_ERROR(void) { return INT32_C(2); }
/* Unfortunatly this seems to kinda not work with some applications, mainly because some set their location AFTER being mapped.
 * We could maybe have a timer or something that would make all configure requests apply this also.
 * Still dont know why, they do this (firefox), wouldnt it look better to do it before? IDK.
 */
double COULDBEFLOATINGGEOM(Client *c)  
                                {
                                    enum { ARRAY_LENGTH = 7 };
                                    enum { SCALE, WEIGHT, WEIGHT_LENGHT };


                                    const double 
                                    SCALE_SIZE_WEIGHTS_LONG[ARRAY_LENGTH][WEIGHT_LENGHT] = 
                                    {
                                        /* SCALE | MULTIPLIER */
                                        {   .95,        1      },
                                        {   .90,       .95     },
                                        {   .80,       .85     },
                                        {   .65,       .50     },
                                        {   .50,       .30     },
                                        {   .25,       .15     },
                                        {   .10,       .0      },
                                    };

                                    const double 
                                    SCALE_SIZE_WEIGHTS_SHORT[ARRAY_LENGTH][WEIGHT_LENGHT] = 
                                    {
                                        /* SCALE | MULTIPLIER */
                                        {   .95,        1      },
                                        {   .90,       .95     },
                                        {   .80,       .90     },
                                        {   .65,       .65     },
                                        {   .50,       .40     },
                                        {   .25,       .25     },
                                        {   .10,       .05     },
                                    };

                                    bool widthBigger = c->w > c->h;
                                    bool sameSize = c->w == c->h;
                                    const double (*width)[WEIGHT_LENGHT];
                                    const double (*height)[WEIGHT_LENGHT];

                                    if(sameSize)
                                    {
                                        width = SCALE_SIZE_WEIGHTS_LONG;
                                        height = SCALE_SIZE_WEIGHTS_LONG;
                                    }
                                    else if(widthBigger)
                                    {
                                        width = SCALE_SIZE_WEIGHTS_LONG;
                                        height = SCALE_SIZE_WEIGHTS_SHORT;
                                    }
                                    else
                                    {
                                        width = SCALE_SIZE_WEIGHTS_SHORT;
                                        height = SCALE_SIZE_WEIGHTS_LONG;
                                    }

                                    int i;
                                    double sizew;
                                    double sizeh;

                                    Monitor *m = c->desktop->mon;

                                    double scorew = 100;
                                    double scoreh = 100;

                                    for(i = 0; i < ARRAY_LENGTH; ++i)
                                    {
                                        sizew = m->mw * width[i][SCALE];

                                        if(c->w > sizew)
                                        {
                                            scorew *= width[i][WEIGHT];
                                            break;
                                        }
                                    }

                                    for(i = 0; i < ARRAY_LENGTH; ++i)
                                    {
                                        sizeh = m->mh * height[i][SCALE];

                                        if(c->h > sizeh)
                                        {
                                            scoreh *= height[i][WEIGHT];
                                            break;
                                        }
                                    }

                                    return (scorew + scoreh) / 2;
                                }
u32 
__COULD__BE__FLOATING__POSITION__FITS(const Client *c, double width_ratio, double height_ratio)
{
    const Monitor *m = c->desktop->mon;

    /* Calculate from center */
    const i32 x = c->x + c->w / 2;
    const i32 y = c->y + c->h / 2;

    /* Calculate center point */
    const i32 cx = m->mx + m->mw / 2;
    const i32 cy = m->my + m->mh / 2;

    /* Calculate Distance */
    const i32 cdx = labs(cx - x);
    const i32 cdy = labs(cy - y);

    const u8 isratiox = (cdx / (float)(cx + !cx)) >= width_ratio;
    const u8 isratioy = (cdy / (float)(cy + !cy)) >= height_ratio;

    /*
    Debug0("----------------------------");
    Debug("(%d, %d)", x, y);
    Debug("(%d, %d)", cx, cy);
    Debug("(%d, %d)", cdx, cdy);
    Debug("(%f, %f)", cdx / (float)cx, cdy / (float)cy);
    Debug("(%u, %u)", isratiox, isratioy);
    Debug("SS: %f", width_ratio);
    */
    return isratiox || isratioy;
}
double COULDBEFLOATINGPOSITION(Client *c)
                                {
                                    enum { ARRAY_LENGTH = 7 };
                                    enum { SCALE, WEIGHT, WEIGHT_LENGHT };

                                    const double 
                                    POSITION_WEIGHTS[ARRAY_LENGTH][WEIGHT_LENGHT] = 
                                    {
                                        /* OFFSET | MULTIPLIER */
                                        {   .50,        1.25   },
                                        {   .40,       .95     },
                                        {   .30,       .70     },
                                        {   .25,       .50     },
                                        {   .10,       .20     },
                                        {   .05,       .07     },
                                        {   .02,       .05     },
                                    };

                                    int i;
                                    double score = 100;

                                    for(i = 0; i < ARRAY_LENGTH; ++i)
                                    {
                                        if(__COULD__BE__FLOATING__POSITION__FITS(c, POSITION_WEIGHTS[i][SCALE], POSITION_WEIGHTS[i][SCALE]))
                                        {
                                            score *= POSITION_WEIGHTS[i][WEIGHT];
                                            break;
                                        }
                                    }

                                    return score;
                                }
double COULDBEFLOATINGHINTS(Client *c)
                                {
                                    double score = 100;
                                    /* This check is mostly for (some) popup windows 
                                     * Mainly those which dont matter, like steams startup display, but are nice to have's.
                                     */
                                    
                                    /* Splash windows are like those "intro" logos they display on some applications, ex: audacity */
                                    if(ISSPLASH(c))
                                    {   
                                        score *= 0.95;
                                        Debug0("Splash Window.");
                                    }
                                    /* Modal dialog boxes are just persistent dialog boxes (aka dont focus anything else as they are important) */
                                    else if(ISMODAL(c))
                                    {   
                                        score *= 0.90;
                                        Debug0("Modal Window.");
                                    }
                                    /* Popup menus see above */
                                    else if(ISPOPUPMENU(c))
                                    {   
                                        score *= 0.90;
                                        Debug0("Popup Menu");
                                    }
                                    /* Dialog boxes are usually floating */
                                    else if(ISDIALOG(c))
                                    {   
                                        score *= 0.90;
                                        Debug0("Dialog Menu");
                                    }
                                    /* Notification boxes like "changed music" things ussualy arent too important, but should still be floating */
                                    else if(ISNOTIFICATION(c))
                                    {   
                                        score *= 0.85;
                                        Debug0("Notification.");
                                    }
                                    /* These are like modal boxes but less important */
                                    else if(ISCOMBO(c))
                                    {   
                                        score *= 0.85;
                                        Debug0("Combo Menu,");
                                    }
                                    /* Above windows ussually are small utility boxes, that shouldnt cover other content completly */
                                    else if(ISABOVE(c))
                                    {   
                                        score *= 0.85;
                                        Debug0("AlwaysOnTop Window detected.");
                                    }
                                    /* This checks for other non dialog types that sort of work like dialog(s) if not maximized. */
                                    else if(ISUTILITY(c))
                                    {   
                                        score *= 0.85;
                                        Debug0("Util Window detected, maybe picture-in-picture?");
                                    }
                                    else if(ISMAXIMIZEDVERT(c) && ISMAXIMIZEDHORZ(c))
                                    {   
                                        score *= .01;
                                        Debug0("Maximized Window detected.");
                                    }
                                    else if(ISNORMAL(c))
                                    {   
                                        score *= .25;
                                        Debug0("Normal Window detected.");
                                    }
                                    else
                                    {
                                        score *= 0.1;
                                        Debug0("Window has no special attributes.");
                                    }

                                    /* No special attributes return */
                                    return score;
                                }

static bool 
__FLOAT__TYPE__EXTRA__CHECKS(Client *c)
{
    const char *const classname = c->classname;
    const char *const instance = c->instancename;

    /* If they dont have a classname/instancename then likely they are single instance windows */
    if(!classname || !instance)
    {   return false;
    }

    /* Some windows do set their classname/instancename but to the same string which means one of the following.
     * A.) Its the main window, which we shouldnt make floating (duh).
     * B.) It has subwindows but again see above.
     * C.) It sets this to all windows and doesnt have any subwindows.
     * D.) (rarely) Its broken, but probably will be fixed later if their developer cares enough.
     */
    return strcasecmp(classname, instance);
}


static bool
__FLOAT__TYPE__IS__FLOATING(
        Client *c,
        double hints,
        double geom,
        double pos
        )
{
    double total = (hints + geom + pos) / 3;

    double MAX_SCORE = 100;

    /* Debug("(h: %lf, g: %lf, p: %lf)", hints, geom, pos); */

    if(total > MAX_SCORE * .5)
    {   return true;
    }

    if(total > MAX_SCORE * .35)
    {
        if(__FLOAT__TYPE__EXTRA__CHECKS(c))
        {   
            Debug0("FLOAT CHECK BYPASSED.");
            return true;
        }
    }

    return false;
}



bool 
SHOULDBEFLOATING(Client *c) 
                                {
                                    /* Note dont check if ISFIXED(c) as games often set that option */
                                    const double htype = COULDBEFLOATINGHINTS(c);
                                    const double gtype = COULDBEFLOATINGGEOM(c);
                                    const double ptype = COULDBEFLOATINGPOSITION(c);

                                    bool ret;

                                    if(USGetSetting(&_cfg, UseLegacyFloatingSystem).data8[0])
                                    {   
                                        ret = LEGACY_SHOULDBEFLOATING(c);
                                        (void)htype;
                                        (void)gtype;
                                        (void)ptype;
                                    }
                                    else
                                    {   ret = __FLOAT__TYPE__IS__FLOATING(c, htype, gtype, ptype);
                                    }

                                    if(!ret)
                                    {   
                                        char *name = "NULL";

                                        if(c->netwmname)
                                        {   name = c->netwmname;
                                        }
                                        else if(c->wmname)
                                        {   name = c->wmname;
                                        }
                                        else if(c->instancename)
                                        {   name = c->instancename;
                                        }
                                        else if(c->classname)
                                        {   name = c->classname;
                                        }

                                        (void)name;
                                        Debug("[%s] Was Not Floating", name);
                                    }
                                    return ret;
                                }
bool
SHOULDCENTER(Client *c)
                        {
                            Monitor *m = c->desktop->mon;
                            /* If the client is floatnig and in a corner center it because ??? */
                            /* Most desktop enviroments do this, and sinec its jarring for windows to spawn wher they said they will
                             * also do this.
                             */
                            if(ISFLOATING(c) || ISFIXED(c))
                            {
                                /* has the client its coords? */
                                if(c->x == m->mx && c->y == m->my)
                                {   return true;
                                }

                                /* leeway some clients calculations have roundring errors*/
                                const int LEEWAY_PX = MAX_ROUNDING_ERROR();
                                
                                bool xleeway = IN_RANGE(c->x, m->mx, LEEWAY_PX);
                                bool yleeway = IN_RANGE(c->y, m->my, LEEWAY_PX);

                                if(xleeway && yleeway)
                                {   return true;
                                }
                            }

                            bool offScreenBeforeX = c->x + WIDTH(c) < m->wx;
                            bool offScreenAfterX = c->x - (WIDTH(c) - c->w) > m->ww;

                            bool offScreenBeforeY = c->y + HEIGHT(c) < m->wy;
                            bool offScreenAfterY = c->y - (HEIGHT(c) - c->h) > m->wh;

                            if(offScreenBeforeX || offScreenAfterX || offScreenBeforeY || offScreenAfterY)
                            {   return true;
                            }

                            return false;
                        }
/* This covers some apps being able to DragWindow/ResizeWindow, in toggle.c
 * (semi-frequently) a user might "accidentally" click on them (me) and basically we dont want that window to be floating because of that user error.
 * So this is a leeway sort function.
 */
u32 SHOULDMAXIMIZE(Client *c)   {
                                    if(DOCKED(c))
                                    {   return 0;
                                    }

                                    Monitor *m = c->desktop->mon;
                                    const i16 wx = m->wx;
                                    const i16 wy = m->my;
                                    const i16 mx = m->mx;
                                    const i16 my = m->my;

                                    const u16 ww = m->ww;
                                    const u16 wh = m->wh;
                                    const u16 mw = m->mw;
                                    const u16 mh = m->mh;

                                    const i16 x = c->x;
                                    const i16 y = c->y;
                                    const u16 w = c->w;
                                    const u16 h = c->h;
                                    const u16 w1 = WIDTH(c);
                                    const u16 h1 = HEIGHT(c);


                                    /* leeway, pixels */
                                    const u8 LWY = MAX_ROUNDING_ERROR();
                                    const u8 iww = IN_RANGE(w, ww, LWY) && IN_RANGE(h, wh, LWY);
                                    const u8 iwb = IN_RANGE(w1, ww, LWY) && IN_RANGE(h1, wh, LWY);
                                    const u8 imw = IN_RANGE(h, mh, LWY) && IN_RANGE(w, mw, LWY);
                                    const u8 imb = IN_RANGE(h1, mh, LWY) && IN_RANGE(w1, mw, LWY);

                                    /* leeway, pixels */
                                    const u8 LWYC = 15;
                                    const u8 iwx = IN_RANGE(x, wx, LWYC) && IN_RANGE(y, wy, LWYC);
                                    const u8 imx = IN_RANGE(x, mx, LWYC) && IN_RANGE(y, my, LWYC);

                                    return
                                        (iwx && (iww || iwb))
                                        ||
                                        (imx && (imw || imb))
                                    ;
                                }

/* used in manage */
u32 DOCKEDINITIAL(Client *c)    {   Monitor *m = c->desktop->mon;
                                    const i16 wx = m->wx;
                                    const i16 wy = m->my;
                                    const i16 mx = m->mx;
                                    const i16 my = m->my;

                                    const u16 ww = m->ww;
                                    const u16 wh = m->wh;
                                    const u16 mw = m->mw;
                                    const u16 mh = m->mh;

                                    const i16 x = c->x;
                                    const i16 y = c->y;
                                    const u16 w = c->w;
                                    const u16 h = c->h;
                                    const u16 w1 = WIDTH(c);
                                    const u16 h1 = HEIGHT(c);

                                    return
                                        ((wx == x) && (wy == y) && (ww == w) && (wh == h))
                                        ||
                                        ((wx == x) && (wy == y) && (ww == w1) && (wh == h1))
                                        ||
                                        ((mx == x) && (my == y) && (mh == h) && (mw == w))
                                        ||
                                        ((mx == x) && (my == y) && (mh == h1) && (mw == w1))
                                        ;
                                }

u32 SHOULDMANAGE(const XCBWindow window) 
                                {
                                    Client *c = wintoclient(window);
                                    XCBWindow already_managed_window = c ? c->win : 0;
                                    XCBWindow already_managed_decor_window = c ? c->decor->win : 0;
                                    const XCBWindow INVALID_WINDOW[] = 
                                    {
                                        XCB_NONE,
                                        _wm.root,
                                        _wm.wmcheckwin,
                                        already_managed_window,
                                        already_managed_decor_window
                                    };

                                    int i;
                                    for(i = 0; i < LENGTH(INVALID_WINDOW); ++i)
                                    {
                                        if(window == INVALID_WINDOW[i])
                                        {   return 0;
                                        }
                                    }

                                    return 1;
                                }

u32 WASDOCKEDVERT(Client *c)    {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->oldy;
                                    const u16 h = OLDHEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }
u32 WASDOCKEDHORZ(Client *c)    {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->oldx;
                                    const u16 w = OLDWIDTH(c);
                                    return (wx == x) && (ww == w);
                                }

u32 WASDOCKED(Client *c)        { return WASDOCKEDVERT(c) && WASDOCKEDHORZ(c); }

u32 DOCKEDVERT(Client *c)       {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->y;
                                    const u16 h = HEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }

u32 DOCKEDHORZ(Client *c)       {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->x;
                                    const u16 w = WIDTH(c);
                                    return (wx == x) && (ww == w);
                                }
u32 DOCKED(Client *c)           { return DOCKEDVERT(c) && DOCKEDHORZ(c); }

u32 ISFIXED(Client *c)          { return (c->minw != 0) && (c->minh != 0) && (c->minw == c->maxw) && (c->minh == c->maxh); }
u32 ISMAXHORZ(Client *c)        { return WIDTH(c) == c->desktop->mon->ww; }
u32 ISMAXVERT(Client *c)        { return HEIGHT(c) == c->desktop->mon->wh; }
u32 ISVISIBLE(Client *c)        { return (c->desktop->mon->desksel == c->desktop || ISSTICKY(c)) && !(ISHIDDEN(c) || ISMAPICONIC(c)); }
/* manage */
u32 CANMANAGE(XCBWindow win, bool allow_unmapped_window, XCBGetWindowAttributes *waattributes, XCBWindowProperty *wastate) {
                    u32 *data = NULL;
                    uint32_t size = 0;
                    int status = 0;

                    enum { NO_FORMAT = 1 };

                    if(wastate)
                    {   
                        data = XCBGetWindowPropertyValue(wastate);
                        status = XCBGetWindowPropertyValueSize(wastate, &size);
                    }

                    if(waattributes)
                    {
                        if(waattributes->override_redirect)
                        {
                            Debug("Override Redirect: [%d]", win);
                            goto NO_MANAGE;
                        }

                        if(waattributes->map_state != XCBIsViewable && !allow_unmapped_window)
                        {   goto NO_MANAGE;
                        }
                    }

                    if(wastate)
                    {
                        if(status != NO_FORMAT && data)
                        {   
                            if(*data != XCB_WINDOW_NORMAL_STATE && *data != XCB_WINDOW_ICONIC_STATE && !allow_unmapped_window)
                            {   goto NO_MANAGE;
                            }
                        }
                    }
                    return 1;
NO_MANAGE:
                    return 0;
                }
