/* MIT License
 *
 * Copyright (c) 2024 Joseph
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __DY__NAMIC__ARRAY__H__
#define __DY__NAMIC__ARRAY__H__

#include <stdint.h>
#include <stdio.h>

#ifdef __G_ARRAY_H__
#error "Using Glib for dynamic array, delete this line to acknowledge this"
#else
#define __G_ARRAY_H__
#endif

/* G stands for generic by the way.
 */
typedef struct GArray32 GArray32;
typedef GArray32 GArray;
/* typedef struct GArray64 GArray64; */
/* UNUSED cause why would you use this? 
 * Encapsulation is useless at this low level. 
 * Even had said that I still use encapsulation, and half the time I only do it for sustainability of the project. 
 * Otherwise why would I?
 */
typedef uint32_t garray_i32;
/* typedef uint64_t garray_i64; */

typedef garray_i32 garray_i;

/* DONOT MODIFY DATA INSIDE.
 *
 */
struct
GArray32
{
    void *data;
    size_t item_size;

    garray_i data_len;
    garray_i data_len_real;
    garray_i base_allocate;
};

#define GARRAY_STATIC_INITIALIZER(ITEM_SIZE) { .data = NULL, .item_size = ITEM_SIZE, .data_len = 0, .data_len_real = 0, .base_allocate = 0 }


/*
struct
GArray64
{
    void *data;
    size_t item_size;

    garray_i64 data_len;
    garray_i64 data_len_real;
    garray_i64 base_allocate;
};
*/

/* Do not use inside data. use functions only
 *
 *
 * RETURN: GArray * on Success
 * RETURN: NULL on Failure.
 */
GArray *
GArrayCreate(
    size_t item_size,
    garray_i base_allocate
    );

/* Do not use inside data. use functions only
 *
 * NOTE: This function will always 'succeed' if array_return is a valid pointer, item_size is greater than 0.
 *
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayCreateFilled(
    GArray *array_return,
    size_t item_size,
    garray_i base_allocate
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
void
GArrayWipe(
    GArray *array
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayResize(
    GArray *array,
    garray_i item_len
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayPushBack(
    GArray *array,
    void *item_cpy
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayPopBack(
    GArray *array
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayReplace(
    GArray *array,
    void *item_cpy,
    garray_i index
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayInsert(
    GArray *array,
    void *item_cpy,
    garray_i index
    );

/*
 * RETURN: EXIT_SUCCESS on Sucesss.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayDelete(
    GArray *array,
    garray_i index
    );

void *
GArrayAt(
        GArray *array,
        garray_i index
        );
int
GArrayAtSafe(
        GArray *array,
        garray_i index,
        void *fill_return
        );


/* This gets the raw array data, that may be required for some library optimizations
 *
 * NOTE: This will only fail if GArray *array is NULL.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
GArrayGetArray(
    GArray *array,
    void **array_return,
    size_t *sizeof_array_return,
    size_t *item_size_return
    );

garray_i
GArrayEnd(
        GArray *array
        );

garray_i
GArrayStart(
        GArray *array
        );

#endif
