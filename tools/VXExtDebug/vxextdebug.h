/* MIT License
 *
 * Copyright (c) 2026- Joseph
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

#ifndef __VXEXTDEBUG__H__
#define __VXEXTDEBUG__H__

#include <assert.h>

#ifdef __GNUC__
    #define INTERNAL __attribute__((visibility("hidden")))
#else
    #define INTERNAL
#endif

enum
VXMExtDebugType
{
    VXMExtDebugINFO,
    VXMExtDebugDEBUG,
    VXMExtDebugWARN,
    VXMExtDebugERROR,
    VXMExtDebugCRITICAL,
};

extern void INTERNAL vxextdebug(enum VXMExtDebugType type, const char *file, const int line, const char *function, const char *fmt, ...);


#ifndef DebugWarn
    #define DebugWarn(...) vxextdebug(VXMExtDebugWARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef DebugCrit
    #define DebugCrit(...) vxextdebug(VXMExtDebugCRITICAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef DebugError
    #define DebugError(...) vxextdebug(VXMExtDebugERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef NDEBUG

    #ifndef Debug
        #define Debug(...) vxextdebug(VXMExtDebugDEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
    #endif

    #ifndef DebugLog
        #define DebugLog(...) vxextdebug(VXMExtDebugINFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
    #endif

    #ifndef Debug0
        #define Debug0(X) Debug("%s", X)
    #endif

    #ifndef ASSERT
        #define ASSERT(expr) (assert(likely(expr)), likely(expr))
    #endif

#else
    #ifndef Debug
        #define Debug(...) ((void)0)
    #endif

    #ifndef DebugLog
        #define DebugLog(...) ((void)0)
    #endif

    #ifndef Debug0
        #define Debug0(X)       ((void)0)
    #endif

    #ifndef ASSERT
        #define ASSERT(expr)       (likely(expr))
    #endif

#endif


#endif
