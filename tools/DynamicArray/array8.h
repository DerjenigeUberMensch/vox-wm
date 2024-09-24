#ifndef __ARRAY__8__H__
#define __ARRAY__8__H__



#include "dynamic_array.h"

typedef struct __GArray__ GArray8;

GArray *
GArrayCreate8(
    uint32_t base_allocate
    );

int
GArrayCreateFilled8(
    GArray *array_return,
    uint32_t base_allocate
    );

void
GArrayWipe8(
    GArray *array
    );

int
GArrayResize8(
    GArray *array,
    uint32_t item_len
    );

int
GArrayPopBack8(
    GArray *array
    );

int
GArrayReplace8(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayInsert8(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayDelete8(
    GArray *array,
    uint32_t index
    );


void *
GArrayAt8(
        GArray *array,
        uint32_t index
        );

int
GArrayAtSafe8(
        GArray *array,
        uint32_t index,
        void *fill_return
        );

uint32_t
GArrayEnd8(
        GArray *array
        );

uint32_t 
GArrayStart8(
        GArray *array
        );

#endif
