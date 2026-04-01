#include <math.h>
#include <string.h>

#include "main.h"

#include "legacy/floating.h"

enum
FloatType
{
    DefinitelyFloating,
    ProbablyFloating,
    CouldBeFloating,
    ProbablyNotFloating,
    DefinitelyNotFloating,

    FLOATINGLAST,
};


static const unsigned int
LEGACY__COULD__BE__FLOATING__GEOM__FITS(Client *c, const float WIDTH_RATIO, const float HEIGHT_RATIO)
{
    const Monitor *m = c->desktop->mon;
    return c->w <= (m->mw * WIDTH_RATIO) || c->h <= (m->mh * HEIGHT_RATIO);
}

static float LEGACY__SIGMOID__SCALING(int i , float k, float z0)
{   return 1.0f / (1.0f + expf(-k * (i - z0)));
}

/* Unfortunatly this seems to kinda not work with some applications, mainly because some set their location AFTER being mapped.
 * We could maybe have a timer or something that would make all configure requests apply this also.
 * Still dont know why, they do this (firefox), wouldnt it look better to do it before? IDK.
 */
static enum FloatType 
LEGACY_COULDBEFLOATINGGEOM(Client *c)  
                                {
                                    const float k = .8f;
                                    const float z0 = 0x0;

                                    float scale;
                                    i32 i;
                                    for(i = 0; i < FLOATINGLAST; ++i)
                                    {   
                                        /* Roughly as follows (k = 1.5f, z0 = 1.5f)
                                         * DefinitelyFloating,      10 %
                                         * ProbablyFloating,        30 %
                                         * CouldBeFloating,         67 %
                                         * ProbablyNotFloating,     90 %
                                         * DefinitelyNotFloating,   97 %
                                         */
                                        scale = LEGACY__SIGMOID__SCALING(i, k, z0);
                                        /* Debug("%f", scale); */
                                        if(LEGACY__COULD__BE__FLOATING__GEOM__FITS(c, scale, scale))
                                        {   return (enum FloatType)i;
                                        }
                                    }
                                    if(ISFIXED(c))
                                    {
                                        const float MIN_W_RATIO = .67f;
                                        if(c->maxw <= c->desktop->mon->mw * MIN_W_RATIO || c->maxh <= c->desktop->mon->mh * MIN_W_RATIO)
                                        {   return ProbablyFloating;
                                        }
                                    }
                                    return ProbablyNotFloating;
                                }
static u32 
LEGACY__COULD__BE__FLOATING__POSITION__FITS(const Client *c, float width_ratio, float height_ratio)
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

static enum FloatType 
LEGACY_COULDBEFLOATINGPOSITION(Client *c)
                                {
                                    const float k = 0.8f;
                                    const float z0 = 3.6f;

                                    i32 i;
                                    float scale;
                                    for(i = 0; i < FLOATINGLAST; ++i)
                                    {
                                        scale = LEGACY__SIGMOID__SCALING(i, k, z0);
                                        if(LEGACY__COULD__BE__FLOATING__POSITION__FITS(c, scale, scale))
                                        {   return (enum FloatType)i;
                                        }
                                    }
                                    return ProbablyNotFloating;
                                }
static enum FloatType 
LEGACY_COULDBEFLOATINGHINTS(Client *c)
                                {
                                    /* This check is mostly for (some) popup windows 
                                     * Mainly those which dont matter, like steams startup display, but are nice to have's.
                                     */
                                    
                                    /* Splash windows are like those "intro" logos they display on some applications, ex: audacity */
                                    if(ISSPLASH(c))
                                    {   
                                        Debug0("Splash Window.");
                                        return ProbablyFloating;
                                    }
                                    /* Modal dialog boxes are just persistent dialog boxes (aka dont focus anything else as they are important) */
                                    else if(ISMODAL(c))
                                    {   
                                        Debug0("Modal Window.");
                                        return ProbablyFloating;
                                    }
                                    /* Popup menus see above */
                                    else if(ISPOPUPMENU(c))
                                    {   
                                        Debug0("Popup Menu");
                                        return ProbablyFloating;
                                    }
                                    /* Dialog boxes are usually floating */
                                    else if(ISDIALOG(c))
                                    {   
                                        Debug0("Dialog Menu");
                                        return ProbablyFloating;
                                    }
                                    /* Notification boxes like "changed music" things ussualy arent too important, but should still be floating */
                                    else if(ISNOTIFICATION(c))
                                    {   
                                        Debug0("Notification.");
                                        return ProbablyFloating;
                                    }
                                    /* These are like modal boxes but less important */
                                    else if(ISCOMBO(c))
                                    {   
                                        Debug0("Combo Menu,");
                                        return DefinitelyFloating;
                                    }
                                    /* Above windows ussually are small utility boxes, that shouldnt cover other content completly */
                                    else if(ISABOVE(c))
                                    {   
                                        Debug0("AlwaysOnTop Window detected.");
                                        return DefinitelyFloating;
                                    }
                                    /* This checks for other non dialog types that sort of work like dialog(s) if not maximized. */
                                    else if(ISUTILITY(c))
                                    {   
                                        Debug0("Util Window detected, maybe picture-in-picture?");
                                        return ProbablyFloating;
                                    }
                                    else if(ISMAXIMIZEDVERT(c) && ISMAXIMIZEDHORZ(c))
                                    {   
                                        Debug0("Maximized Window detected.");
                                        return DefinitelyNotFloating;
                                    }
                                    else if(ISNORMAL(c))
                                    {   
                                        Debug0("Normal Window detected.");
                                        return ProbablyNotFloating;
                                    }
                                    Debug0("Window has no special attributes.");
                                    /* No special attributes return */
                                    return ProbablyNotFloating;
                                }

static bool 
LEGACY__FLOAT__TYPE__EXTRA__CHECKS(Client *c)
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
    return strcmp(classname, instance);
}


static bool
LEGACY__FLOAT__TYPE__IS__FLOATING(
        Client *c,
        const enum FloatType hints, 
        const enum FloatType geom, 
        const enum FloatType pos
        )
{
    enum FloatType ret = DefinitelyNotFloating;
    switch(hints)
    {
        case DefinitelyFloating:
        {
            switch(geom)
            {
                case DefinitelyFloating:
                case ProbablyFloating:
                case CouldBeFloating:
                    ret = DefinitelyFloating;
                    break;
                case ProbablyNotFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;
                case DefinitelyNotFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            ret = CouldBeFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;

                    /* unused */
                case FLOATINGLAST:
                    break;
            }
            break;
        }
        case ProbablyFloating:
        {
            switch(geom)
            {
                case DefinitelyFloating:
                case ProbablyFloating:
                    ret = DefinitelyFloating;
                case CouldBeFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                    }
                    break;
                case ProbablyNotFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                    }
                    break;
                case DefinitelyNotFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                            ret = ProbablyFloating;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyFloating:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;

                    /* unused */
                case FLOATINGLAST:
                    break;
            }
        }
        case CouldBeFloating:
        {
            switch(geom)
            {
                case DefinitelyFloating:
                    ret = DefinitelyFloating;
                    break;
                case ProbablyFloating:
                    switch(pos)
                    {   
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = ProbablyFloating; 
                            }
                            break;
                    }
                    break;
                case CouldBeFloating:
                    switch(pos)
                    {   
                        case DefinitelyFloating:
                        case ProbablyFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                    }
                    break;
                case ProbablyNotFloating:
                    switch(pos)
                    {   
                        case DefinitelyFloating:
                            ret = ProbablyFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyFloating:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;
                case DefinitelyNotFloating:
                    switch(pos)
                    {   
                        case DefinitelyFloating:
                            ret = CouldBeFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;
                    /* unused */
                case FLOATINGLAST:
                    break;

            }
            break;
        }
        case ProbablyNotFloating:
        {
            switch(geom)
            {
                case DefinitelyFloating:
                    ret = ProbablyFloating;
                case ProbablyFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                            ret = CouldBeFloating;
                            break;
                            /* unused */
                        case FLOATINGLAST:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;
                case CouldBeFloating:
                    switch(pos)
                    {
                        case FLOATINGLAST:
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            break;
                    }
                case ProbablyNotFloating:
                    switch(pos)
                    {
                        case FLOATINGLAST:
                        case DefinitelyFloating:
                        case ProbablyFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                            break;
                    }
                    break;
                case DefinitelyNotFloating:
                    switch(pos)
                    {   
                        case FLOATINGLAST:
                        case DefinitelyFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                        case ProbablyFloating:
                        case DefinitelyNotFloating:
                        case ProbablyNotFloating:
                        case CouldBeFloating:
                            break;
                    }
                    break;
                case FLOATINGLAST:
                    break;
            }
            break;
        }
        case DefinitelyNotFloating:
        {
            switch(geom)
            {
                case DefinitelyFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                        case CouldBeFloating:
                            ret = CouldBeFloating;
                            break;
                        case DefinitelyNotFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                        case ProbablyNotFloating:
                        case FLOATINGLAST:
                        default:
                            break;
                    }
                case ProbablyFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                        case ProbablyFloating:
                            ret = CouldBeFloating;
                            break;
                        case CouldBeFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                        case DefinitelyNotFloating:
                        case ProbablyNotFloating:
                        case FLOATINGLAST:
                        default:
                            break;
                    }
                    break;
                case CouldBeFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                            ret = CouldBeFloating;
                            break;
                            /* unused */
                        case ProbablyFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                        case CouldBeFloating:
                        case DefinitelyNotFloating:
                        case ProbablyNotFloating:
                        case FLOATINGLAST:
                        default:
                            break;
                    }
                    break;
                case ProbablyNotFloating:
                case DefinitelyNotFloating:
                    switch(pos)
                    {
                        case DefinitelyFloating:
                            if(LEGACY__FLOAT__TYPE__EXTRA__CHECKS(c))
                            {   ret = CouldBeFloating; 
                            }
                            break;
                        case ProbablyFloating:
                        case CouldBeFloating:
                        case ProbablyNotFloating:
                        case DefinitelyNotFloating:
                        case FLOATINGLAST:
                        default:
                            break;
                    }
                    break;
                    /* unused */
                case FLOATINGLAST:
                    break;
            }
        }

        /* unused */
        case FLOATINGLAST:
        {   break;
        }
    }
    const Monitor *m = c->desktop->mon;
    Debug("Float state: %d", ret);
    /* UNUSED DUE TO unreliablity.
    switch(ret)
    {
        case DefinitelyFloating:
            break;
        case ProbablyFloating:
            break;
        case CouldBeFloating:
            break;
        case ProbablyNotFloating:
            break;
        case DefinitelyNotFloating:
            break;
        case FLOATINGLAST:
        default:
            break;
    }
    */
    /* check if in the corner */
    if(c->x == m->mx && c->y == m->my)
    {   ret = DefinitelyNotFloating;
    }
    Debug("Is float: %s", ret != DefinitelyNotFloating ? "true" : "false");
    return ret != DefinitelyNotFloating;   
}



bool 
LEGACY_SHOULDBEFLOATING(Client *c) 
                                {
                                    /* Note dont check if ISFIXED(c) as games often set that option */
                                    const enum FloatType htype = LEGACY_COULDBEFLOATINGHINTS(c);
                                    const enum FloatType gtype = LEGACY_COULDBEFLOATINGGEOM(c);
                                    const enum FloatType ptype = LEGACY_COULDBEFLOATINGPOSITION(c);

                                    bool ret = LEGACY__FLOAT__TYPE__IS__FLOATING(c, htype, gtype, ptype);
                                    Debug("(%d, %d, %d)", htype, gtype, ptype);
                                    if(!ret)
                                    {   Debug("[%s] Was Not Floating", c->wmname ? c->wmname : c->netwmname ? c->netwmname : "NULL");
                                    }
                                    return ret;
                                }
