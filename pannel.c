#include "pannel.h"

#include <stdlib.h>
#include <stdio.h>


Pannel *
PannelCreate(
        int16_t x,
        int16_t y,
        uint16_t w,
        uint16_t h
        )
{
    Pannel *ret = malloc(sizeof(Pannel ));
    if(ret)
    {
        ret->x = x;
        ret->y = y;
        ret->w = w;
        ret->h = h;
    }
    return ret;
}

void
PannelMap(
        Pannel *pannel
        )
{   
}

void
PannelUnmap(
        Pannel * pannel
        )
{

}

