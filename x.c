#include <stdio.h>
#include <string.h>

#include "x.h"



void
getnamefromreply(XCBWindowProperty *namerep, char **str_return)
{
    if(namerep)
    {
        if(namerep->type && namerep->length > 0)
        {
            const size_t offset = XCBGetPropertyValueSize(namerep);
            char *str = XCBGetPropertyValue(namerep);
            char *nstr = malloc(sizeof(char) * offset + sizeof(char));
            if(nstr)
            {   
                memcpy(nstr, str, offset);
                memcpy(nstr + offset, "\0", sizeof(char));
            }
            *str_return = nstr;
        }
    }
}

uint32_t *
geticonprop(XCBWindowProperty *iconreply)
{
    const uint8_t WIDTH_INDEX = 0;
    const uint8_t HEIGHT_INDEX = 1;
    const uint8_t MIN_WIDTH = 1;
    const uint8_t MIN_HEIGHT = 1;
    const uint8_t MAX_WIDTH = 255;
    const uint8_t MAX_HEIGHT = 255;
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
        if(iconreply->format != 32)
        {   /* DEBUG("Icon format is not standard, icon may be corrupt. %d", iconreply->format); */
        }
        uint32_t *icon = XCBGetPropertyValue(iconreply);
        size_t size = XCBGetPropertyValueSize(iconreply);
        uint32_t length = XCBGetPropertyValueLength(iconreply, sizeof(uint32_t));
        if(length >= MIN_ICON_DATA_SIZE)
        {   
            uint64_t i = 0;
            uint64_t wi = WIDTH_INDEX;
            uint32_t hi = HEIGHT_INDEX;
            /* get the biggest size */
            while(i + 2 < length)
            {                                               /* bounds check */
                if(icon[i + WIDTH_INDEX] > icon[wi] && icon[i + WIDTH_INDEX] <= MAX_WIDTH && length - i >= icon[i])
                {   wi = i;
                }
                                                                            /* bounds check */
                if(icon[i + HEIGHT_INDEX] > icon[hi] && icon[i + HEIGHT_INDEX] <= MAX_HEIGHT && length - i >= icon[i + 1])
                {   hi = i + HEIGHT_INDEX;
                }
                i += icon[i] + icon[i + 1];
                /* this covers use fucking up and the +plus 2 for width and height offset */
                i += 2;
            }
            size_t sz = sizeof(uint32_t) * icon[wi] * icon[hi] + sizeof(uint32_t) * 2;
            if(sz <= size)
            {   size = sz;
            }
            ret = malloc(size);
            if(ret)
            {   memcpy(ret, &icon[wi], size);
            }
        }
    }
    return ret;
}

