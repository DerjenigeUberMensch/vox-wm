/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

#include "util.h"

void *
ecalloc(size_t nmemb, size_t size)
{
    void *p;

    p = calloc(nmemb, size);
    return p;
    if(p) return p;
    /* exit if calloc failed Likely OS is out of memory */
    DEBUG("%s","FATAL: FAILED TO CALLOC MEMORY");
    exit(1);
}
char *
smprintf(char *fmt, ...)
{
    va_list fmtargs;
    char *ret;
    int len;

    va_start(fmtargs, fmt);
    len = vsnprintf(NULL, 0, fmt, fmtargs);
    va_end(fmtargs);

    ret = malloc(++len);
    if(!ret)
    {
        perror("smprintf Failed: ");
        return "NULL";
    }
    va_start(fmtargs, fmt);
    vsnprintf(ret, len, fmt, fmtargs);
    va_end(fmtargs);
    return ret;
}

void
debug(char *fmt, ...)
{
#ifdef ENABLE_DEBUG
    static char str[1024];
    memset(str, 0, sizeof(char) * 1024);
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, vsnprintf(NULL, 0, fmt, args), fmt, args);
    va_end(args);
    perror(str);
    fprintf(stdout, "%s\n", str);
#endif
}

unsigned int
UIHash(unsigned int x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

unsigned int
UIUnHash(unsigned int x)
{
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}

uint64_t 
UI64Hash(uint64_t x)
{
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

uint64_t
UI64UnHash(uint64_t x)
{
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return x;
}

double 
functime(void (*_timefunction)(void))
{
    clock_t start, end;
    start = clock();
    _timefunction();
    end = clock();

    return ((double)(end - start)) / CLOCKS_PER_SEC;
}
