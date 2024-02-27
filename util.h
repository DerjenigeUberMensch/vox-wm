#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdio.h>
/* See LICENSE file for copyright and license details. */

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))

#define ENABLE_DEBUGGING            0       /* enable debug messages for gdb via debug()                */

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
char *smprintf(char *fmt, ...);
void debug(char *fmt, ...);
unsigned int UIHash(unsigned int x);
unsigned int UIUnHash(unsigned int x);
uint64_t UI64Hash(uint64_t x);
uint64_t UI64UnHash(uint64_t x);


double functime(void (*_timefunction)(void));

#define ASSUME(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)





#endif
