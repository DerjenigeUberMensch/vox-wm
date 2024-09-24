# GArray

A small (300 lines) dynamic array.

## Example
```C

#include <stdio.h>
#include <stdlib.h>

#include "dynamic_array.h"


int 
main(void) 
{
    uint32_t item_size = sizeof(int);
    uint32_t initial_size = 10;
    uint32_t i;

    GArray *array = GArrayCreate(item_size, initial_size);
    if (array == NULL) 
    {   return EXIT_FAILURE;
    }

    /* make all data 0's to show it works */
    for(i = GArrayStart(array); i < GArrayEnd(array); ++i)
    {   GArrayReplace(array, NULL, i);
    }

    int value = 42;
    if (GArrayPushBack(array, &value) != EXIT_SUCCESS) 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }


    int *p = (int *)GArrayAt(array, 0);

    if (p) 
    {   printf("Previous Value at index 0: %d\n", *p);
    } 
    else 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }

    int new_value = 99;
    if (GArrayReplace(array, &new_value, 0) != EXIT_SUCCESS) 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }

    p = (int *)GArrayAt(array, 0);
    if (p) 
    {   printf("New value at index 0: %d\n", *p);
    } 
    else 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }

    int insert_value = 55;
    if (GArrayInsert(array, &insert_value, 1) != EXIT_SUCCESS) 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }

    printf("Value at index %d: %d\n", 1, *(int *)GArrayAt(array, 1));
    printf("Deleting Index %d\n", 1);

    if (GArrayDelete(array, 1) != EXIT_SUCCESS) 
    {
        GArrayWipe(array);
        free(array);
        return EXIT_FAILURE;
    }

    for (i = GArrayStart(array); i < GArrayEnd(array); ++i) 
    {
        p = (int *)GArrayAt(array, i);
        if (p) 
        {   printf("Value at index %d: %d\n", i, *p);
        }
    }

    GArrayWipe(array);
    free(array);
    return EXIT_SUCCESS;
}

