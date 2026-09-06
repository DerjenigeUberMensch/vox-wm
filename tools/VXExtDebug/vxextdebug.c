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
#include "vxextdebug.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>


#define RESET_ALL     "\x1b[0m"
#define COLOR_RESET   "\x1b[39m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_CYAN    "\x1b[96m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"

static struct winsize vxext_w = {0};
static pthread_mutex_t vxext_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t vxext_init = PTHREAD_ONCE_INIT;
static int vxext_no_winsize = 0;

static void
vxextdebug_lock(void)
{   pthread_mutex_lock(&vxext_lock);
}

static void
vxextdebug_unlock(void)
{   pthread_mutex_unlock(&vxext_lock);
}

static void
vxextdebuginit_impl(void)
{
    int status;

    status = ioctl(STDOUT_FILENO, TIOCGWINSZ, &vxext_w);

    if(status == -1)
    {   vxext_no_winsize = 1;
    }
}

static void
vxextdebuginit(void)
{
    static volatile int intialized = 0;
    int status;

    if(intialized)
    {   return;
    }

    vxextdebug_lock();

    if(intialized)
    {   
        vxextdebug_unlock();       
        return;
    }

    status = pthread_once(&vxext_init, vxextdebuginit_impl);

    /* fallback if the world explodes */
    if(status != 0)
    {   vxextdebuginit_impl();
    }

    intialized = 1;
    vxextdebug_unlock();
}


void
vxextdebug(enum VXMExtDebugType type, const char *file, const int line, const char *func, const char *fmt, ...)
{
    static int log_count = 0;

    vxextdebuginit();

    time_t t = time(NULL);
    struct tm tm = {0};

    if(!localtime_r(&t, &tm))
    {   memset(&tm, 0, sizeof(tm));
    }

    const char* typestr;
    const char* color;
    const char* colormsg;

    switch(type) 
    {
        case VXMExtDebugINFO:     typestr = "INFO ";    color = COLOR_GREEN;   colormsg = "";           break;
        case VXMExtDebugDEBUG:    typestr = "DEBUG";    color = COLOR_BLUE;    colormsg = "";           break;
        case VXMExtDebugWARN:     typestr = "WARN ";    color = COLOR_YELLOW;  colormsg = COLOR_YELLOW; break;
        case VXMExtDebugERROR:    typestr = "ERROR";    color = COLOR_RED;     colormsg = COLOR_RED;    break; 
        case VXMExtDebugCRITICAL: typestr = "CRIT ";    color = COLOR_RED;     colormsg = COLOR_RED;    break;
        default:                  typestr = "(\?\?\?)"; color = COLOR_MAGENTA; colormsg = "";           break;
    }

    enum LogCols 
    {
        TIME_W = 8,
        TYPE_W = 8,
        FILE_W = 20,
        LINE_W = 6,
        FUNC_W = 20,
        MIN_EVENT_W = 10,
        FALLBACK_EVENT_W = 40,
        PADDING = 5
    };

    int event_indent = TIME_W + TYPE_W + FILE_W + LINE_W + FUNC_W + PADDING;
    int event_w;

    if(vxext_no_winsize)
    {   event_w = FALLBACK_EVENT_W;
    }
    else
    {   event_w = vxext_w.ws_col - event_indent;
    }

    if (event_w < MIN_EVENT_W)
    {   event_w = MIN_EVENT_W;
    }

    enum { MSG_SIZE_MAX = 1024 };

    char msg[MSG_SIZE_MAX] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    vxextdebug_lock();

    if(!isatty(STDERR_FILENO))
    {     
        color = "";
        colormsg = "";
    }

    ++log_count;

    if (!vxext_no_winsize && vxext_w.ws_row > 2 && log_count % (vxext_w.ws_row - 2) == 1) 
    {
        fprintf(stderr, 
                "%-*s "
                "%-*s "
                "%-*s "
                "%-*s "
                "%-*s "
                "%s\n",
                TIME_W, "Time",
                TYPE_W, "Type",
                FILE_W, "File",
                LINE_W, "Line",
                FUNC_W, "Function",
                "Event"
                );
    }


    fprintf(stderr, 
                    "%02d:%02d:%02d "
                    "%s %s %s "
                    "%-*s "
                    "%-*d "
                    "%-*s "
                    "%s",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            color, typestr, COLOR_RESET,
            FILE_W, file,
            LINE_W, line,
            FUNC_W, func,
            colormsg
            );

    /* extra... */
    size_t msg_len = strlen(msg);
    size_t pos = 0;

    while (pos < msg_len)
    {
        size_t remaining = msg_len - pos;
        size_t n = remaining > (size_t)event_w ? (size_t)event_w : remaining;

        /*
         * If this isn't the first line, indent it
         * to the beginning of the Event column.
         */
        if (pos != 0)
        {   fprintf(stderr, "%*s%s", event_indent, "", colormsg);
        }

        fprintf(stderr,
                "%.*s%s\n",
                (int)n,
                msg + pos,
                RESET_ALL);

        pos += n;
    }

    /*
     * Make sure an empty message still produces a line.
     */
    if (msg_len == 0)
    {   fprintf(stderr, "%s\n", RESET_ALL);
    }

    vxextdebug_unlock();
}

#undef COLOR_RESET
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_MAGENTA
#undef RESET_ALL
#undef COLOR_CYAN
