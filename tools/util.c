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






