#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/resource.h>

#include "VXExtDebug/vxextdebug.h"
#include "thpool/thpool.h"
#include "threading.h"
#include "util.h"
#include "args.h"


threadpool __thread__pool = NULL;
pthread_mutex_t __thread_mutex = PTHREAD_MUTEX_INITIALIZER;

int
InitThreading(void)
{
    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   
        pthread_mutex_unlock(&__thread_mutex);
        return EXIT_SUCCESS;
    }

    u32 aloc_threads = 4;
    const u32 MAX_THREADS_SYS = 64;  /* Anything past 64 threads wouldnt be particularly necessary. */
    const u32 MAX_THREADS_ABOVE_ALL = 2048;
    u64 MAX_THREADS_REAL = MAX_THREADS_SYS;

    struct rlimit r;
    
    if(getrlimit(RLIMIT_NPROC, &r) == 0)
    {   MAX_THREADS_REAL = r.rlim_cur;
    }

    long cores = sysconf(_SC_NPROCESSORS_ONLN);

    if(cores > aloc_threads)
    {   aloc_threads = MAX(cores - 1, aloc_threads);
    }

    aloc_threads = MIN(aloc_threads, MAX_THREADS_SYS);

    const ArgOpt *opt = WMCheckArg(WMArgThreads);

    if(opt && opt->found && opt->argument_return)
    {
        char *end;
        long val = strtol(opt->argument_return, &end, 10);

        if(*end == '\0')
        {   
            aloc_threads = MAX(val, 1);

            if(MAX_THREADS_REAL > LONG_MAX)
            {   MAX_THREADS_REAL = LONG_MAX;
            }

            if(MAX_THREADS_REAL > MAX_THREADS_ABOVE_ALL)
            {   MAX_THREADS_REAL = MAX_THREADS_ABOVE_ALL;
            }

            aloc_threads = MIN(aloc_threads, MAX_THREADS_REAL);

            DebugLog("Threads manually set to -> %ld", aloc_threads);
        }
        else
        {   DebugLog("Failed to parse %s", opt->argument_return);
        }

    }

    __thread__pool = thpool_init(aloc_threads);

    if(__thread__pool)
    {   
        pthread_mutex_unlock(&__thread_mutex);
        return EXIT_SUCCESS;
    }
    pthread_mutex_unlock(&__thread_mutex);

    return EXIT_FAILURE;
}

static void
ThreadingWorker(void *arg)
{
    ThreadWorkItem *work = (ThreadWorkItem *)arg;

    if(!ASSERT(work))
    {   return;
    }

    if(!ASSERT(work->func))
    {   goto CLEANUP;   
    }

    work->func(&work->arg);
    
CLEANUP:
    if(work->promise)
    {   ResolveTPromise(work->promise, NULL);
    }

    free(work);
}

int
ThreadingAddWork(void (*function)(Generic *arg), Generic *arg, TPromise *optional_tpromise_to_use)
{
    if(!ASSERT(function))
    {   return EXIT_FAILURE;
    }

    int status = EXIT_FAILURE;

    ThreadWorkItem *arg_real = malloc(sizeof(*arg_real));

    if(!arg_real)
    {   return EXIT_FAILURE;
    }

    arg_real->arg = arg ? *arg : (Generic){0};
    arg_real->promise = optional_tpromise_to_use;
    arg_real->func = function;

    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   status = thpool_add_work(__thread__pool, ThreadingWorker, (void *)arg_real);
    }

    pthread_mutex_unlock(&__thread_mutex);

    if(!status)
    {   return EXIT_SUCCESS;
    }

    free(arg_real);

    return EXIT_FAILURE;
}

void 
ThreadingWaitAll(void)
{   
    pthread_mutex_lock(&__thread_mutex);
    if(__thread__pool)
    {   thpool_wait(__thread__pool);
    }
    pthread_mutex_unlock(&__thread_mutex);
}

void 
ThreadingDestroy(void)
{   
    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   
        /* this can be a memory leak sometimes... But currently there is not another good way to halt threads without breaking stuff */
        thpool_destroy(__thread__pool);
        __thread__pool = NULL;
    }

    pthread_mutex_unlock(&__thread_mutex);
}

void 
ThreadingPause(void)
{   
    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   thpool_pause(__thread__pool);
    }

    pthread_mutex_unlock(&__thread_mutex);
}

void 
ThreadingResume(void)
{   
    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   thpool_resume(__thread__pool);
    }

    pthread_mutex_unlock(&__thread_mutex);
}

int 
ThreadingGetNumWorking(void)
{
    int ret = 0;

    pthread_mutex_lock(&__thread_mutex);

    if(__thread__pool)
    {   ret = thpool_num_threads_working(__thread__pool);
    }

    pthread_mutex_unlock(&__thread_mutex);

    return ret;
}

bool
ThreadingUsesThreads(void)
{   
    bool use_threads = false;

    pthread_mutex_lock(&__thread_mutex);

    use_threads = __thread__pool != NULL;

    pthread_mutex_unlock(&__thread_mutex);

    return use_threads;
}
