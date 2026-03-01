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

#include <stdlib.h>
#include <string.h>

#include "garray.h"


GArray *
GArrayCreate(
    size_t item_size,
    garray_i base_allocate
    )
{
    GArray *ret = malloc(sizeof(GArray));

    if(ret)
    {
        uint8_t status = GArrayCreateFilled(ret, item_size, base_allocate);
        if(status == EXIT_FAILURE)
        {
            GArrayWipe(ret);
            free(ret);
            ret = NULL;
        }
    }

    return ret;
}

int
GArrayCreateFilled(
    GArray *array_return,
    size_t item_size,
    garray_i base_allocate
    )
{
    if(!array_return || !item_size)
    {   return EXIT_FAILURE;
    }

    array_return->item_size = item_size;
    array_return->data = NULL;
    array_return->data_len = 0;
    array_return->data_len_real = 0;
    array_return->base_allocate = base_allocate;

    GArrayResize(array_return, base_allocate);

    /* replace head index */
    GArrayMoveHead(array_return, 0);

    return EXIT_SUCCESS;
}

void
GArrayWipe(
    GArray *array
    )
{
    if(!array)
    {   return;
    }

    GArrayResize(array, 0);
}

void
GArrayClear(
        GArray *array
        )
{
    if(!array)
    {   return;
    }

    GArrayResize(array, array->base_allocate);
    GArrayMoveHead(array, 0);
}

int
GArrayMoveHead(
        GArray *array,
        garray_i index_to_move_to
        )
{
    if(index_to_move_to > array->data_len)
    {   return EXIT_FAILURE;
    }

    array->data_len = index_to_move_to;

    return EXIT_SUCCESS;
}

int
GArrayResize(
    GArray *array,
    garray_i item_len
    )
{
    if(!array)
    {   return EXIT_FAILURE;
    }

    if(array->data_len == item_len)
    {   return EXIT_SUCCESS;
    }

    if(item_len > SIZE_MAX / array->item_size)
    {   return EXIT_FAILURE;
    }

    if(item_len == 0)
    {   
        free(array->data);

        array->data = NULL;
        array->data_len = 0;
        array->data_len_real = 0;
    }
    else if(!array->data)
    {
        garray_i length;

        if(array->base_allocate > item_len)
        {   length = array->base_allocate;
        }
        else
        {   length = item_len;
        }

        array->data = malloc(array->item_size * length);

        if(!array->data)
        {   return EXIT_FAILURE;
        }

        array->data_len_real = length;
    }
    else
    {
        garray_i length;

        if(array->data_len_real < item_len)
        {   
            garray_i new_cap = array->data_len_real;

            if (new_cap == 0)
            {   new_cap = array->base_allocate ? array->base_allocate : 1;
            }

            while (new_cap < item_len)
            {   new_cap *= 2;
            }

            length = new_cap;
        }
        else
        {
            garray_i used_space = array->data_len_real - array->base_allocate;
            float REDUCE_THRESHOLD = .225f;
            float EXTRA_KEPT = 1.5f;

            if(used_space * REDUCE_THRESHOLD >= array->data_len)
            {   
                length = array->data_len * EXTRA_KEPT;

                if(length < array->base_allocate)
                {   length = array->base_allocate;
                }
            }
            else
            {   goto END;
            }
        }

        void *rec = realloc(array->data, array->item_size * length);

        if(!rec && length)
        {   return EXIT_FAILURE;
        }

        array->data = rec;
        array->data_len_real = length;
    }
END:
    array->data_len = item_len;

    return EXIT_SUCCESS;
}

int
GArrayPushBack(
    GArray *array,
    void *item_cpy
    )
{
    if(!array || !item_cpy)
    {   return EXIT_FAILURE;
    }

    uint8_t status = GArrayResize(array, array->data_len + 1);

    if(status == EXIT_SUCCESS)
    {   
        if(array->data)
        {
            uint8_t *data = array->data;
            uint8_t *dest = data + (array->data_len - 1) * array->item_size;
            uint8_t *src = item_cpy;
            garray_i size = array->item_size;

            memmove(dest, src, size);
        }
    }
    return status;
}

int
GArrayPopBack(
    GArray *array
    )
{
    if(!array)
    {   return EXIT_FAILURE;
    }
    if(array->data)
    {   
        /* make sure no underflow */
        if(array->data_len)
        {   return GArrayResize(array, array->data_len - 1);
        }
    }
    return EXIT_SUCCESS;
}

int
GArrayReplace(
    GArray *array,
    void *item_cpy,
    garray_i index
    )
{
    if(!array)
    {   return EXIT_FAILURE;
    }
    if(index >= array->data_len)
    {   return EXIT_FAILURE;
    }

    garray_i size = array->item_size;
    uint8_t *data = array->data;
    uint8_t *dest = data + index * size;
    uint8_t *src = item_cpy;

    if(item_cpy)
    {   memmove(dest, src, size);
    }
    else
    {   memset(dest, 0, size);
    }
    return EXIT_SUCCESS;
}

int
GArrayInsert(
    GArray *array,
    void *item_cpy,
    garray_i index
    )
{
    if(!array)
    {   return EXIT_FAILURE;
    }

    if(index > array->data_len)
    {   return EXIT_FAILURE;
    }

    uint8_t status = GArrayResize(array, array->data_len + 1);

    if(status == EXIT_SUCCESS)
    {   
        int isEnd = (array->data_len == index) ? 0 : 1;
        garray_i size = array->item_size;

        uint8_t *data = array->data;
        uint8_t *dest = data + (index + 1) * size;
        uint8_t *src = data + index * size;

        garray_i move_size = (array->data_len - index - isEnd) * size;

        if(data)
        {
            memmove(dest, src, move_size);
            if(item_cpy)
            {   memmove(src, item_cpy, size);
            }
            else
            {   memset(src, 0, size);
            }
        }
    }

    return status;
}

int
GArrayDelete(GArray *array, garray_i index) 
{ 
    if(!array) 
    {   return EXIT_FAILURE;
    }

    if(index >= array->data_len)
    {   return EXIT_FAILURE;
    }

    const garray_i size = array->item_size;
    const garray_i BYTES_MOVE = (array->data_len - index - 1) * size;

    uint8_t *data = array->data;
    uint8_t *src = data + (size * (index + 1));
    uint8_t *dest = data + (size * index);

    /* Check if last so no invalid memove 
     * No check for underflow as that wouldnt matter, and would always fail index anyways.
     */
    if(index < array->data_len - 1)
    {   memmove(dest, src, BYTES_MOVE);
    }

    return GArrayResize(array, array->data_len - 1);
}


void *
GArrayAt(
        GArray *array,
        garray_i index
        )
{
    if(!array || array->data_len <= index)
    {   return NULL;
    }

    return (uint8_t *)array->data + (index * array->item_size);
}

int
GArrayAtSafe(
        GArray *array,
        garray_i index,
        void *fill_return
        )
{
    void *data = GArrayAt(array, index);
    int ret = EXIT_FAILURE;
    if(fill_return)
    {
        if(data)
        {
            memmove(fill_return, data, array->item_size);
            ret = EXIT_SUCCESS;
        }
    }
    return ret;
}

int
GArrayGetArray(
    GArray *array,
    void **array_return,
    size_t *data_len,
    size_t *sizeof_array_return,
    size_t *item_size_return
    )
{
    if(!array)
    {   return EXIT_FAILURE;
    }

    if(data_len)
    {   *data_len = (GArrayEnd(array) - GArrayStart(array));
    }

    if(array_return)
    {   *array_return = array->data;
    }

    if(sizeof_array_return)
    {   *sizeof_array_return = array->data_len * array->item_size;
    }

    if(item_size_return)
    {   *item_size_return = array->item_size;
    }

    return EXIT_SUCCESS;
}


garray_i
GArrayEnd(
        GArray *array
        )
{
    if(array)
    {   return array->data_len;
    }

    return 0;
}

garray_i
GArrayStart(
        GArray *array
        )
{   return 0;
}

