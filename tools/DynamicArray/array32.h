#ifndef __ARRAY__32__H__
#define __ARRAY__32__H__

#include "dynamic_array.h"

typedef struct __GArray__ GArray32;


GArray *
GArrayCreate32(
    uint32_t base_allocate
    );

int
GArrayCreateFilled32(
    GArray *array_return,
    uint32_t base_allocate
    );

void
GArrayWipe32(
    GArray *array
    );

int
GArrayResize32(
    GArray *array,
    uint32_t item_len
    );

int
GArrayPopBack32(
    GArray *array
    );

int
GArrayReplace32(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayInsert32(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayDelete32(
    GArray *array,
    uint32_t index
    );


void *
GArrayAt32(
        GArray *array,
        uint32_t index
        );

int
GArrayAtSafe32(
        GArray *array,
        uint32_t index,
        void *fill_return
        );

uint32_t
GArrayEnd32(
        GArray *array
        );

uint32_t 
GArrayStart32(
        GArray *array
        );


#endif
