/* MIT License
 *
 * Copyright (c) 2024- Joseph
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

#include "util.h"

bool 
memempty(void *mem, size_t size)
{
    size_t i;

    for(i = 0; i < size; ++i)
    {
        char *num = (char *)mem + i;

        if(*num != 0)
        {   return false;
        }
    }

    return true;
}

bool 
memnonempty(void *mem, size_t size)
{   return !memempty(mem, size);
}

bool 
memfilled(void *mem, size_t size)
{
    size_t i;

    for(i = 0; i < size; ++i)
    {   
        char *num = (char *)mem + i;
        if(*num != ~0)
        {   return false;
        }
    }
    return true;
}

uint32_t 
PACK_ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b) 
{
    int byteorder = GET_BYTE_ORDER();

    (void)ASSERT(GET_BYTE_ORDER() != 0);

    switch(byteorder)
    {
        /* Big Endian */
        case 1234: return b | (g << 8) | (r << 16) | (a << 24);
        /* Little Endian */
        case 4321: return (a << 24) | (r << 16) | (g << 8) | b;
        /* PDP Endian */
        case 3412: return (r << 16) | (a << 24) | (b) | (g << 8);
    }

    return 0;
}

void
UNPACK_ARGB(uint32_t argb, uint8_t *a_return, uint8_t *r_return, uint8_t *g_return, uint8_t *b_return)
{
    int byteorder = GET_BYTE_ORDER();

    (void)ASSERT(GET_BYTE_ORDER() != 0);

    switch (byteorder)
    {
        /* Big Endian */
        case 1234: 
            if(b_return) 
            {   *b_return = argb & 0xFF;
            }
            if(g_return) 
            {   *g_return = (argb >> 8) & 0xFF;
            }
            if(r_return) 
            {   *r_return = (argb >> 16) & 0xFF;
            }
            if(a_return) 
            {   *a_return = (argb >> 24) & 0xFF;
            }
        break;

        /* Little Endian */
        case 4321: 
            if(a_return) 
            {   *a_return = (argb >> 24) & 0xFF;
            }
            if(r_return) 
            {   *r_return = (argb >> 16) & 0xFF;
            }
            if(g_return) 
            {   *g_return = (argb >> 8) & 0xFF;
            }
            if(b_return) 
            {   *b_return = argb & 0xFF;
            }
            break;
        /* PDP Endian */
        case 3412: 
            if(r_return) 
            {   *r_return = (argb >> 16) & 0xFF;
            }
            if(a_return) 
            {   *a_return = (argb >> 24) & 0xFF;
            }
            if(b_return) 
            {   *b_return = argb & 0xFF;
            }
            if(g_return) 
            {   *g_return = (argb >> 8) & 0xFF;
            }
            break;
        default:
            return;
    }
}

int
GET_BYTE_ORDER(void)
{
    #if defined(__GNUC__) || defined(__clang__)
        return __BYTE_ORDER__;
    #elif defined(_MSC_VER)
        #if defined(_WIN32)
            return 1234;
        #endif
    #elif defined(__INTEL__COMPILER)
        #if defined(__GNUC__)
            return __BYTE_ORDER__;
        #endif
    #endif

    uint32_t x = 0x11223344;
    unsigned char *p = (unsigned char *)&x;

    if (p[0] == 0x44 && p[1] == 0x33 && p[2] == 0x22 && p[3] == 0x11)
    {   return 1234;
    }
    else if (p[0] == 0x11 && p[1] == 0x22 && p[2] == 0x33 && p[3] == 0x44)
    {   return 4321;
    }
    else if (p[0] == 0x33 && p[1] == 0x44 && p[2] == 0x11 && p[3] == 0x22)
    {   return 3412;
    }

    return 0;
}

void _Breakpoint(void) { volatile int *e = 0; if(e != (volatile int *)1) { e = (volatile int *)3; } (void)e; }


#ifdef __linux__ 

#include <pthread.h>

int 
PTHREAD_INIT_RECURSIVE_MUTEX(pthread_mutex_t *mutex)
{
    pthread_mutexattr_t attr;
    int ret;

    ret = pthread_mutexattr_init(&attr);

    if (ret)
    {   return ret;
    }

    ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if (ret) 
    {
        pthread_mutexattr_destroy(&attr);
        return ret;
    }

    ret = pthread_mutex_init(mutex, &attr);

    pthread_mutexattr_destroy(&attr);

    return ret;
}

#endif






