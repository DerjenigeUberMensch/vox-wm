/* See LICENSE file for copyright and license details. */
#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef int8_t byte;
typedef uint8_t ubyte;

typedef float f32;
typedef double f64;
typedef long double f128;

typedef union Generic Generic;
typedef union ARGB ARGB;

union Generic
{
    void *datav;
    void **datavl;

    int8_t data8[64];
    int16_t data16[32];
    int32_t data32[16];
    int64_t data64[8];

    float dataf[16];
    double datad[8];
    long double datadd[4];  /* compiler specified but should be at most 128 bits */
};

/* ORDER.
 * BLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24)
 */
union ARGB
{
#if __BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
    uint8_t a;  /* Alpha value */
    uint8_t r;  /* Red Value   */
    uint8_t g;  /* Green Value */
    uint8_t b;  /* Blue Value  */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    uint8_t b;  /* Blue Value  */
    uint8_t g;  /* Green Value */
    uint8_t r;  /* Red Value   */
    uint8_t a;  /* Alpha value */
#else
    /* 
     * NO SUPPORTED ENDIAN TYPE.
     * If you are using PDP_ENDIAN you might have to manually shift the values yourself.
     */
    #error "No supported endian type. If you are using PDP_ENDIAN you might have to manually shift the values yourself."
#endif
    uint32_t argb;  /* ARGB 32bit value */
};



#define M_STRINGIFY(x)  #x
#define M_CONCAT(a, b)  a##b

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))
#define FREE(F)                 do              \
                                {               \
                                    free(F);    \
                                    F = NULL;   \
                                } while(0)
#define CLEARFLAG(FLAGS, FLAG)            (((FLAGS) &= (~FLAG)))
#define SETFLAG(FLAGS, FLAG, STATE)       do                                        \
                                          {                                         \
                                              ((FLAGS) &= (~FLAG));                 \
                                              ((FLAGS) |= ((FLAG * !!(STATE))));    \
                                          } while(0)
#define FLAGSET(FLAGS, FLAG)            (((FLAGS & FLAG)))

#define DIE(fmt, ...)           do { fprintf(stderr, "[%s:%d] by %s(): " fmt "\n", __FILE__,__LINE__,__func__,__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define DIECAT(fmt, ...)        do { fprintf(stderr, "[%s:%d] by %s()\n"                \
                                    "________________________________\n"                \
                                    "|                    /)        |\n"                \
                                    "|           /\\___/\\ ((         |\n"              \
                                    "|           \\`@_@'/  ))        "  "   " fmt "\n"  \
                                    "|           {_:Y:.}_//         |\n"                \
                                    "|-----------{_}^-'{_}----------|\n"                \
                                    "\n", __FILE__,__LINE__,__func__,__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#ifdef ENABLE_DEBUG
#define DEBUG(fmt, ...) (fprintf(stderr, "[%s:%d] by %s(): " fmt "\n", __FILE__,__LINE__,__func__,__VA_ARGS__))
#define DEBUG0(X) (fprintf(stderr, "[%s:%d] by %s(): " X "\n", __FILE__, __LINE__, __func__))
#else
#define DEBUG(fmt, ...) ((void)0)
#define DEBUG0(X)       ((void)0)
#endif

/* gcc */
#ifdef __GNUC__
#define ASM(X)                          (__asm__(X))
#define asm __asm__
#define __HOT__                          __attribute__((hot))
#define __COLD__                        __attribute__((cold))
#define NOINLINE                        __attribute__ ((noinline))
#elif __clang__
#define ASM(X)                          (__asm__(X))
#define asm __asm__
#define __HOT__                         __attribute__((hot))
#define __COLD__                        __attribute__((cold))
#define NOINLINE                        __attribute__ ((noinline))

#else
#define ASM(X)                          ((void)X)
#defube asm(X)                          ((void)X)
#define __HOT__
#define __COLD__
#define NOINLINE                        

#endif




void *ecalloc(size_t nmemb, size_t size);
char *smprintf(char *fmt, ...);
void debug(char *fmt, ...);

double functime(void (*_timefunction)(void));






#ifndef False
#define False 0
#endif

#ifndef True
#define True 1
#endif

#ifndef None
#define None 0
#endif

#ifndef Nill
#define Nill ((void *)0)
#endif

#define ASSUME(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)

/* Original implementation: Simon Tatham 
 * https://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html 
 */
/*
 * Copyright (c) 2001 Simon Tatham.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL SIMON TATHAM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define MERGE_SORT_LINKED_LIST(TYPE, CMP_FUNC, HEAD, TAIL, NEXT, PREV, IS_DOUBLE, IS_CIRCULAR)\
                        do                                                              \
                        {                                                               \
                            TYPE *__p, *__q, *__e, *__tail, *__oldhead;                 \
                            TYPE *__list;                                               \
                            uint32_t __insize, __nmerges, __psize, __qsize, __i;        \
                            __insize = 1;                                               \
                            __list = HEAD;                                              \
                            while(1)                                                    \
                            {                                                           \
                                __p = __list;                                           \
                                if(IS_CIRCULAR)                                         \
                                {   __oldhead = __list;                                 \
                                }                                                       \
                                __list = NULL;                                          \
                                __tail = NULL;                                          \
                                /* count number of merges we do in this pass */         \
                                __nmerges = 0;                                          \
                                while(__p)                                              \
                                {   /* there exists a merge to be done */               \
                                    ++__nmerges;                                        \
                                    /* step `insize' places along from p */             \
                                    __q = __p;                                          \
                                    __psize = 0;                                        \
                                    for(__i = 0; __i < __insize; ++__i)                 \
                                    {                                                   \
                                        ++__psize;                                      \
                                        if(IS_CIRCULAR)                                 \
                                        {   __q = (__q->NEXT == __oldhead ? NULL : __q->NEXT);  \
                                        }                                               \
                                        else                                            \
                                        {   __q = __q->NEXT;                            \
                                        }                                               \
                                        if(!__q)                                        \
                                        {   break;                                      \
                                        }                                               \
                                    }                                                   \
                                    /* if q hasn't fallen off end,                      \
                                     * we have two lists to merge                       \
                                     */                                                 \
                                    __qsize = __insize;                                 \
                                                                                        \
                                    /* now we have two lists; merge them */             \
                                    while(__psize > 0 || (__qsize > 0 && __q))          \
                                    {                                                   \
                                        /* decide where the next element                \
                                         * of merge comes from p or q                   \
                                         */                                             \
                                        if(__psize == 0)                                \
                                        {                                               \
                                            /* p is empty; e must come from q. */       \
                                            __e = __q; __q = __q->NEXT; --__qsize;      \
                                            if(IS_CIRCULAR && __q == __oldhead)         \
                                            {   __q = NULL;                             \
                                            }                                           \
                                        }                                               \
                                        else if(__qsize == 0 || !__q)                   \
                                        {                                               \
                                            /* q is empty; e must come from p. */       \
                                            __e = __p; __p = __p->NEXT; --__psize;      \
                                            if(IS_CIRCULAR && __p == __oldhead)         \
                                            {   __p = NULL;                             \
                                            }                                           \
                                        }                                               \
                                        else if(CMP_FUNC(__p, __q) <= 0)                \
                                        {                                               \
                                            /* First element of p is lower (or same);   \
                                             * e must come from p.                      \
                                             */                                         \
                                            __e = __p; __p = __p->NEXT; --__psize;      \
                                            if(IS_CIRCULAR && __p == __oldhead)         \
                                            {   __p = NULL;                             \
                                            }                                           \
                                        }                                               \
                                        else                                            \
                                        {                                               \
                                            /* First element of q is lower;             \
                                             * e must come from q.                      \
                                             */                                         \
                                            __e = __q; __q = __q->NEXT; --__qsize;      \
                                            if(IS_CIRCULAR && __q == __oldhead)         \
                                            {   __q = NULL;                             \
                                            }                                           \
                                        }                                               \
                                        /* add the next element to the merged list */   \
                                        if(__tail)                                      \
                                        {   __tail->NEXT = __e;                         \
                                        }                                               \
                                        else                                            \
                                        {   __list = __e;                               \
                                        }                                               \
                                        if(IS_DOUBLE)                                   \
                                        {                                               \
                                            /* Maintain reverse pointers                \
                                             * in a doubly linked list.                 \
                                             */                                         \
                                            __e->PREV = __tail;                         \
                                        }                                               \
                                        __tail = __e;                                   \
                                    }                                                   \
                                    /* now p has stepped `insize' places along,         \
                                     * and q has too                                    \
                                     */                                                 \
                                    __p = __q;                                          \
                                }                                                       \
                                if(IS_CIRCULAR)                                         \
                                {                                                       \
                                    if(__tail)                                          \
                                    {   __tail->NEXT = __list;                          \
                                    }                                                   \
                                    if(IS_DOUBLE)                                       \
                                    {   __list->PREV = __tail;                          \
                                    }                                                   \
                                }                                                       \
                                else                                                    \
                                {                                                       \
                                    if(__tail)                                          \
                                    {   __tail->NEXT = NULL;                            \
                                    }                                                   \
                                }                                                       \
                                /* If we have done only one merge, we're finished.*/    \
                                if(__nmerges <= 1)                                      \
                                {                                                       \
                                    HEAD = __list;                                      \
                                    TAIL = __tail;                                      \
                                    break;                                              \
                                }                                                       \
                                /* Otherwise repeat, merging lists twice the size */    \
                                __insize *= 2;                                          \
                            }                                                           \
                        } while(0)


#endif
