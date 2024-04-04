/* See LICENSE file for copyright and license details. */
#ifndef UTIL_H_
#define UTIL_H_

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
#define DEBUG0(X) ((void)0)
#endif

/* gcc */
#ifdef __GNUC__
#define ASM(X)                          (__asm__(X))
#define NOINLINE                        __attribute__ ((noinline))


#elif __clang__

#define ASM(X)                          (__asm__(X))
#define NOINLINE                        __attribute__ ((noinline))

#else
#define ASM(X)                          ((void)X)
#define NOINLINE                        (())
#endif




void *ecalloc(size_t nmemb, size_t size);
char *smprintf(char *fmt, ...);
unsigned int UIHash(unsigned int x);
unsigned int UIUnHash(unsigned int x);
uint64_t UI64Hash(uint64_t x);
uint64_t UI64UnHash(uint64_t x);


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





#endif
