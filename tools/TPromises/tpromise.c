/* MIT License
 *
 * Copyright (c) 2024 Joseph
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
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tpromise.h"

static int
make_deadline(struct timespec *ts, double ms)
{
    if(ms < 0.0)
    {   return EINVAL;
    }

    if(clock_gettime(CLOCK_MONOTONIC, ts) == -1)
    {   return errno;
    }

    long long ns = (long long)(ms * 1000000.0);

    ts->tv_sec  += ns / 1000000000LL;
    ts->tv_nsec += ns % 1000000000LL;

    if(ts->tv_nsec >= 1000000000L)
    {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000L;
    }

    return 0;
}

void *
AwaitTPromise(
    TPromise *promise
    )
{
    void *data;

    pthread_mutex_lock(&promise->mutex);

    while(!promise->resolved)
    {   pthread_cond_wait(&promise->cond, &promise->mutex);
    }

    data = promise->data;

    pthread_mutex_unlock(&promise->mutex);

    return data;
}

void *
AwaitTPromiseTimeout(
    TPromise *promise,
    double timeout_ms
    )
{
    void *data = NULL;
    struct timespec deadline;

    int status = make_deadline(&deadline, timeout_ms);

    if(status)
    {   return NULL;
    }

    pthread_mutex_lock(&promise->mutex);

    while(!promise->resolved)
    {
        status = pthread_cond_timedwait(&promise->cond, &promise->mutex, &deadline);

        if(status == ETIMEDOUT)
        {   break;
        }

        if(status != 0)
        {   break;
        }
    }

    if(promise->resolved)
    {   data = promise->data;
    }

    pthread_mutex_unlock(&promise->mutex);

    return data;
}

void
ClearTPromise(
    TPromise *promise
    )
{
    if(!promise)
    {   return;
    }

    pthread_mutex_lock(&promise->mutex);

    while(!promise->resolved)
    {   pthread_cond_wait(&promise->cond, &promise->mutex);
    }

    promise->data = NULL;
    promise->resolved = 0;

    pthread_mutex_unlock(&promise->mutex);
}

void
DestroyTPromise(
    TPromise *promise
    )
{
    if(!promise)
    {   return;
    }

    DestroyTPromiseFilled(promise);

    free(promise);
}

void
DestroyTPromiseFilled(
        TPromise *promise
        )
{
    if(!promise)
    {   return;
    }

    pthread_mutex_destroy(&promise->mutex);
    pthread_cond_destroy(&promise->cond);
}

int
IsTPromiseDone(
    TPromise *promise
    )
{
    int promise_done = 0;

    if(!promise)
    {   return promise_done;
    }

    pthread_mutex_lock(&promise->mutex);
    if(promise->resolved)
    {   promise_done = 1;
    }
    pthread_mutex_unlock(&promise->mutex);
    
    return promise_done;
}

TPromise *
NewTPromise(
    void
    )
{
    TPromise *promise = malloc(sizeof(*promise));
    if(promise)
    {
        int status = NewTPromiseFilled(promise);
        if(status != EXIT_SUCCESS)
        {   
            free(promise);
            promise = NULL;
        }
    }
    return promise;
}

int
NewTPromiseFilled(
    TPromise *promise_return
    )
{
    if(!promise_return)
    {   return EXIT_FAILURE;
    }

    int status;

    /* check time */
    struct timespec ts;

    status = clock_gettime(CLOCK_MONOTONIC, &ts);

    if(status == -1)
    {   return EXIT_FAILURE;
    }

    promise_return->data = NULL;
    promise_return->resolved = 0;

    status = pthread_mutex_init(&promise_return->mutex, NULL);

    if(status)
        return EXIT_FAILURE;

    pthread_condattr_t attr;

    status = pthread_condattr_init(&attr);

    if(status)
    {
        pthread_mutex_destroy(&promise_return->mutex);
        return EXIT_FAILURE;
    }

    status = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

    if(status)
    {
        pthread_condattr_destroy(&attr);
        pthread_mutex_destroy(&promise_return->mutex);
        return EXIT_FAILURE;
    }

    status = pthread_cond_init(&promise_return->cond, &attr);

    pthread_condattr_destroy(&attr);

    if(status)
    {
        pthread_mutex_destroy(&promise_return->mutex);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

void
ResolveTPromise(
    TPromise *promise, 
    void *data
    )
{
    pthread_mutex_lock(&promise->mutex);

    if(!promise->resolved)
    {   
        promise->data = data;
        promise->resolved = 1;
        pthread_cond_broadcast(&promise->cond);
    }

    pthread_mutex_unlock(&promise->mutex);
}
