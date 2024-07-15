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
        DEBUG("Length:  [%u]", namerep->length);
        DEBUG("Type:    [%u]", namerep->type);
        DEBUG("Format:  [%u]", namerep->format);
        DEBUG("Sequence:[%u]", namerep->sequence);
        DEBUG("VALENGTH:[%u]", namerep->value_len);
        DEBUG("BytesAft:[%u]", namerep->bytes_after);
        DEBUG("Resptype:[%u]", namerep->response_type);
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





XCBCookie
GetTransientCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetTransientForHintCookie(display, win);
}

XCBCookie
GetWindowStateCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
GetWindowTypeCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
GetSizeHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMNormalHintsCookie(display, win);
}

XCBCookie
GetWMHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMHintsCookie(display, win);
}

XCBCookie
GetWMClassCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMClassCookie(display, win);
}

XCBCookie
GetWMProtocolCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMProtocolsCookie(display, win, wmatom[WMProtocols]);
}

XCBCookie
GetStrutCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_LENGTH = 4;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
GetStrutpCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_P_LENGTH = 12;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
GetNetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
}

XCBCookie
GetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
}

XCBCookie
GetPidCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetPidCookie(display, win, netatom[NetWMPid]);
}

XCBCookie
GetIconCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ANY);
}

XCBCookie
GetMotifHintsCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t MOTIF_WM_HINT_LENGTH = 5;
    return XCBGetWindowPropertyCookie(display, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
}


char *
GetAtomNameQuick(XCBDisplay *display, XCBAtom atom)
{
    XCBCookie cookie;
    XCBAtomName *rep;
    cookie = XCBGetAtomNameCookie(display, atom);
    rep = XCBGetAtomNameReply(display, cookie);
    DEBUG("%u", atom);
    char *buff = NULL;
    if(rep->name_len)
    {   buff = malloc(sizeof(char) * rep->name_len + 1);
    }
    if(buff)
    {   
        memcpy(buff, xcb_get_atom_name_name(rep), rep->name_len);
        buff[rep->name_len] = '\0';
    }
    free(rep);
    return buff;
}
