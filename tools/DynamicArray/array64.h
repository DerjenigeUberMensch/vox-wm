#ifndef __ARRAY__64__H__
#define __ARRAY__64__H__

#include "dynamic_array.h"

typedef struct __GArray__ Garray64;


GArray *
GArrayCreate64(
    uint32_t base_allocate
    );

int
GArrayCreateFilled64(
    GArray *array_return,
    uint32_t base_allocate
    );

void
GArrayWipe64(
    GArray *array
    );

int
GArrayResize64(
    GArray *array,
    uint32_t item_len
    );

int
GArrayPopBack64(
    GArray *array
    );

int
GArrayReplace64(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayInsert64(
    GArray *array,
    void *item_cpy,
    uint32_t index
    );

int
GArrayDelete64(
    GArray *array,
    uint32_t index
    );


void *
GArrayAt64(
        GArray *array,
        uint32_t index
        );

int
GArrayAtSafe64(
        GArray *array,
        uint32_t index,
        void *fill_return
        );

uint32_t
GArrayEnd64(
        GArray *array
        );

uint32_t 
GArrayStart64(
        GArray *array
        );


#endif
