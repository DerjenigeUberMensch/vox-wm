#include <stdio.h>
#include <string.h>

#include "x.h"

#include "util.h"


#include "XCB-TRL/xcb_winutil.h"

extern XCBAtom netatom[];

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
        DEBUG("Length:  [%u]", namerep->length);
        DEBUG("Type:    [%u]", namerep->type);
        DEBUG("Format:  [%u]", namerep->format);
        DEBUG("Sequence:[%u]", namerep->sequence);
        DEBUG("VALENGTH:[%u]", namerep->value_len);
        DEBUG("BytesAft:[%u]", namerep->bytes_after);
        DEBUG("Resptype:[%u]", namerep->response_type);
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

