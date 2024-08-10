#include <stdio.h>
#include <string.h>

#include "x.h"

#include "util.h"


#include "XCB-TRL/xcb_winutil.h"

extern XCBAtom netatom[];
extern XCBAtom wmatom[];
extern XCBAtom motifatom;

uint8_t
checksticky(int64_t x)
{
    /* _NET_WM_DESKTOP
     * https://specifications.freedesktop.org/wm-spec/latest/
     */
    return (x & 0xFFFFFFFF) | ((uint32_t)x == UINT32_MAX) | ((unsigned int)x == ~0)
        /* probably not but failsafe's */
    | ((uint32_t)x == (uint32_t)~0) | ((int32_t)x == -1) | ((uint32_t)x == (uint32_t) -1);
}

char *
getnamefromreply(XCBWindowProperty *namerep)
{
    char *nstr = NULL;
    if(namerep && (namerep->type == XCB_ATOM_STRING || namerep->type == netatom[NetUtf8String]))
    {
        /* debug against stack smashing */
        /*
        Debug("Length:  [%u]", namerep->length);
        Debug("Type:    [%u]", namerep->type);
        Debug("Format:  [%u]", namerep->format);
        Debug("Sequence:[%u]", namerep->sequence);
        Debug("VALENGTH:[%u]", namerep->value_len);
        Debug("BytesAft:[%u]", namerep->bytes_after);
        Debug("Resptype:[%u]", namerep->response_type);
        */
        uint32_t offset = 0;

        XCBGetPropertyValueSize(namerep, &offset);
        char *str = XCBGetPropertyValue(namerep);
        nstr = malloc(sizeof(char) * offset + sizeof(char));
        if(nstr)
        {   
            if(offset)
            {   memcpy(nstr, str, offset);
            }
            nstr[offset] = '\0';
        }
    }
    return nstr;
}

uint32_t *
geticonprop(XCBWindowProperty *iconreply)
{
    const uint8_t WIDTH_INDEX = 0;
    const uint8_t HEIGHT_INDEX = 1;
    const uint8_t MIN_WIDTH = 1;
    const uint8_t MIN_HEIGHT = 1;
    const uint8_t MIN_ICON_DATA_SIZE = (MIN_WIDTH + MIN_HEIGHT) * 2;     /* times 2 cause the first and second index are the size */

    uint32_t *ret = NULL;
    if(iconreply && iconreply->type == XCB_ATOM_CARDINAL)
    {
        uint32_t *icon = XCBGetPropertyValue(iconreply);
        uint32_t length = 0;
        XCBGetPropertyValueLength(iconreply, sizeof(uint32_t), &length);
        if(length >= MIN_ICON_DATA_SIZE)
        {   
            uint64_t i = 0;
            uint64_t wi = WIDTH_INDEX;
            uint64_t hi = HEIGHT_INDEX;
            /* get the biggest size */
            uint8_t bigger = 0;
            uint8_t inrange = 0;

            /* some icons dont work really that well with what they are told */
            while(i + 2 < length)
            {        
                /* assume icon broken */
                if(!icon[wi] || !icon[hi])
                {   break;
                }
                /* jmp to next size(s) */
                /* bounds check */
                bigger = icon[i + WIDTH_INDEX] >= icon[wi];
                inrange = length - i >= icon[i + WIDTH_INDEX];

                if(bigger && inrange)
                {   wi = i + WIDTH_INDEX;
                }

                bigger = icon[i + HEIGHT_INDEX] >= icon[hi];
                inrange = length - i >= icon[i + HEIGHT_INDEX];

                if(bigger && inrange)
                {   hi = i + HEIGHT_INDEX;
                }

                i += icon[i + WIDTH_INDEX] * icon[i + HEIGHT_INDEX];
                /* jmp to next WIDTH_INDEX/HEIGHT_INDEX */
                i += 2;
            }
            size_t sz = sizeof(uint32_t) * icon[wi] * icon[hi] + sizeof(uint32_t) * 2;
            ret = malloc(sz);
            if(ret)
            {   memcpy(ret, &icon[wi], sz);
            }
        }
    }
    return ret;
}



void
XCBSetAtomState(XCBDisplay *display, XCBWindow win, XCBAtom type, XCBAtom atom, XCBWindowProperty *prop, u8 _delete)
{ 
    void *data = NULL;
    u32 len = 0;
    u32 propmode = XCB_PROP_MODE_REPLACE;
    if(prop)
    {
        XCBAtom *atoms = XCBGetPropertyValue(prop);
        uint32_t ATOM_LENGTH = 0;
        XCBGetPropertyValueLength(prop, sizeof(XCBAtom), &ATOM_LENGTH);

        u32 i;
        u32 offset = 0;
        u8 set = 0;
        for(i = 0; i < ATOM_LENGTH; ++i)
        {
            if(atoms[i] == atom)
            {   
                offset = i;
                set = 1;
                break;
            }
        }

        if(set)
        {
            if(_delete)
            {
                /* this gets optimized to memmove, cool!
                 * GCC v14.1.1 -O3
                 */
                /* this is a bottleneck on debug builds, 
                 * But not on release as this is optimized as a memmove.
                 */
                for(i = offset; i < ATOM_LENGTH - 1; ++i)
                {   atoms[i] = atoms[i + 1];
                }
                data = atoms;
                len = ATOM_LENGTH - 1;
            }
            else  /* atom already exists do nothing */
            {   return;
            }
        }
        else
        {
            if(_delete)     /* prop not found mark as already deleted */
            {   return;
            }
            else    /* set propmode to append cause we didnt find it */
            {   
                propmode = XCB_PROP_MODE_APPEND;
                len = 1;
                data = &atom;
            }
        }
    }
    else
    {   
        len = 1;
        data = &atom;
    }
    XCBChangeProperty(display, win, type, XCB_ATOM_ATOM, 32, propmode, (const char *)data, len);
}

char *
GetAtomNameQuick(XCBDisplay *display, XCBAtom atom)
{
    XCBCookie cookie;
    XCBAtomName *rep;
    cookie = XCBGetAtomNameCookie(display, atom);
    rep = XCBGetAtomNameReply(display, cookie);
    Debug("%u", atom);
    char *buff = NULL;
    if(rep)
    {
        const u32 len = xcb_get_atom_name_name_length(rep);
        const size_t size = sizeof(char) * len + sizeof(char);
        if(len)
        {   buff = malloc(size);
        }
        if(buff)
        {   
            memcpy(buff, xcb_get_atom_name_name(rep), len);
            buff[len] = '\0';
        }
        free(rep);
    }
    return buff;
}
