#ifndef __ARRAY__16__H__
#define __ARRAY__16__H__


#include "dynamic_array.h"

typedef struct __GArray__ Garray16;


GArray *
GArrayCreate16(
    uint32_t base_allocate
    );

int
GArrayCreateFilled16(
    GArray *array_return,
    uint32_t base_allocate
    );

void
GArrayWipe16(
    GArray *array
    );

int
GArrayResize16(
    GArray *array,
    uint32_t item_len
    );

int
GArrayPopBack16(
    GArray *array
    );

int
GArrayReplace16(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayInsert16(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayDelete16(
    GArray *array,
    uint32_t index
    );


void *
GArrayAt16(
        GArray *array,
        uint32_t index
        );

int
GArrayAtSafe16(
        GArray *array,
        uint32_t index,
        void *fill_return
        );

uint32_t
GArrayEnd16(
        GArray *array
        );

uint32_t 
GArrayStart16(
        GArray *array
        );


#endif
