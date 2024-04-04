/*
 * NOT TO BE RELEASED ON FINAL BUILDS.
 *
 *
 *
 *
 * This is mostly for debugging certain values without clogging up gdb.
 * But can still be used as a status bar
 */


#include <stdio.h>
#include <stdarg.h>



#include "bar.h"

void
writebar(char *fmt, ...)
{
    static FILE *bar = NULL;
    if(!bar)
    {   bar = popen("lemonbar -p", "w");
    }
    if(!bar)
    {   return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(bar, fmt, args);
    fprintf(bar, "\n");
    vfprintf(stdout, fmt, args);
    fflush(bar);
    va_end(args);
}
