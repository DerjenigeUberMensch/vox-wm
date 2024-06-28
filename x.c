#include <stdio.h>
#include <string.h>

#include "x.h"

#include "util.h"


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
    if(namerep)
    {
        if(namerep->type && namerep->length > 0)
        {
            const size_t offset = XCBGetPropertyValueSize(namerep);
            char *str = XCBGetPropertyValue(namerep);
            nstr = malloc(sizeof(char) * offset + sizeof(char));
            if(nstr)
            {   
                memcpy(nstr, str, offset);
                memcpy(nstr + offset, "\0", sizeof(char));
            }
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
    if(iconreply)
    {
        if(iconreply->format == 0)
        {   
            if(XCBGetPropertyValueSize(iconreply))
            {   /* DEBUG0("Icon has no format, icon may be corrupt."); */
            }
            else
            {   
                /* DEBUG0("No icon."); */
            }
            return ret;
        }
        if(iconreply->type != XCB_ATOM_CARDINAL)
        {   /* DEBUG0("Icon has wierd atom format"); */
        }
        if(iconreply->format != 32)
        {   /* DEBUG("Icon format is not standard, icon may be corrupt. %d", iconreply->format); */
        }
        uint32_t *icon = XCBGetPropertyValue(iconreply);
        uint32_t length = XCBGetPropertyValueLength(iconreply, sizeof(uint32_t));
        if(length >= MIN_ICON_DATA_SIZE)
        {   
            uint64_t i = 0;
            uint64_t wi = WIDTH_INDEX;
            uint32_t hi = HEIGHT_INDEX;
            /* get the biggest size */
            uint8_t bigger = 0;
            uint8_t inrange = 0;
            while(i + 2 < length)
            {        
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
                /* jmp to next size(s) */
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

