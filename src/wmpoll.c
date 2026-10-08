#include "GArray/garray.h"
#include "args.h"
#include "main.h"
#include "util.h"
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "wmpoll.h"

GArray wm_poll_fds;
GArray wm_poll_events;
GArray wm_poll_revents;
GArray wm_poll_callbacks;
GArray wm_poll_args;
GArray wm_poll_buff;

sig_atomic_t wm_poll_exit = 0;

enum { PIPE_READ = 0, PIPE_WRITE = 1, PIPE_LAST };

/* pipe... */
static int wake_pipe[PIPE_LAST];
static int has_pipe = 0;

static void
WMPipeHandler(int fd, int revents, Generic arg)
{
    if(!(revents & POLLIN))
    {   return;
    }

    enum { BUFF_SIZE = 64 * 2 };

    char buff[BUFF_SIZE];

    while(read(fd, buff, sizeof(buff)) > 0);
}

void
WMPollInit(int NUM_OF_POLL_FDS)
{   
    enum { PIPE_FD = 1 };

    int status;

    status = pipe(wake_pipe);

    if(status != 0)
    {   has_pipe = 0;
    }
    else
    {   
        NUM_OF_POLL_FDS += PIPE_FD;
        has_pipe = 1;
    }

    status = GArrayCreateFilled(&wm_poll_buff, UPOLL_UNIT_SIZE, NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_buff array");
    }

    status = GArrayCreateFilled(&wm_poll_fds, sizeof(int), NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_fds array");
    }

    status = GArrayCreateFilled(&wm_poll_events, sizeof(int), NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_events array");
    }

    status = GArrayCreateFilled(&wm_poll_revents, sizeof(int), NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_revents array");
    }

    status = GArrayCreateFilled(&wm_poll_callbacks, sizeof(void (*)(int fd, int revents, Generic arg)), NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_callbacks array");
    }

    status = GArrayCreateFilled(&wm_poll_args, sizeof(Generic), NUM_OF_POLL_FDS);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to create wm_poll_args array");
    }

    if(has_pipe)
    {   WMPollAddFD(wake_pipe[PIPE_READ], POLLIN, WMPipeHandler, (Generic){0});
    }
}

void 
WMPollAddFD(int fd, int events, void (*callback)(int fd, int events, Generic arg), Generic arg)
{
    garray_i fd_end = GArrayEnd(&wm_poll_fds);
    garray_i events_end = GArrayEnd(&wm_poll_events);
    garray_i revents_end = GArrayEnd(&wm_poll_revents);
    garray_i callbacks_end = GArrayEnd(&wm_poll_callbacks);
    garray_i args_end = GArrayEnd(&wm_poll_args);
    garray_i buff_end = GArrayEnd(&wm_poll_buff);

    if(fd_end != events_end || fd_end != callbacks_end || fd_end != args_end || fd_end != buff_end || fd_end != revents_end)
    {   DIECAT("%s", "Inconsistent array sizes");
    }

    if(!ASSERT(callback))
    {   
        DebugError("Invalid callback");
        return;
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_fds, &fd) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append fd to wm_poll_fds array");
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_events, &events) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append events to wm_poll_events array");
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_revents, NULL) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append revents to wm_poll_revents array");
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_callbacks, &callback) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append callback to wm_poll_callbacks array");
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_args, &arg) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append arg to wm_poll_args array");
    }

    if(!ASSERT(GArrayPushBack(&wm_poll_buff, NULL) == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to append NULL to wm_poll_buff array");
    }
}

int
WMPollRun(void)
{
    void *poll_fds = NULL;
    size_t poll_fds_size = 0;
    size_t poll_fds_len = 0;
    size_t unused = 0;

    void *poll_events = NULL;
    size_t poll_events_size = 0;
    size_t poll_events_len = 0;

    void *poll_revents = NULL;
    size_t poll_revents_size = 0;
    size_t poll_revents_len = 0;

    void *poll_buff = NULL;
    size_t poll_buff_size = 0;
    size_t poll_buff_len = 0;

    int status;

    status = GArrayGetArray(&wm_poll_fds, &poll_fds, &poll_fds_len, &unused, &poll_fds_size);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to get array from wm_poll_fds");
    }

    if(!ASSERT(poll_fds_size == sizeof(int)))
    {   DIECAT("%s", "Unexpected size of poll_fds array");
    }

    status = GArrayGetArray(&wm_poll_events, &poll_events, &poll_events_len, &unused, &poll_events_size);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to get array from wm_poll_events");
    }

    if(!ASSERT(poll_events_size == sizeof(int)))
    {   DIECAT("%s", "Unexpected size of poll_events array");
    }

    status = GArrayGetArray(&wm_poll_revents, &poll_revents, &poll_revents_len, &unused, &poll_revents_size);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to get array from wm_poll_revents");
    }

    if(!ASSERT(poll_revents_size == sizeof(int)))
    {   DIECAT("%s", "Unexpected size of poll_revents array");
    }

    status = GArrayGetArray(&wm_poll_buff, &poll_buff, &poll_buff_len, &unused, &poll_buff_size);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   DIECAT("%s", "Failed to get array from wm_poll_buff");
    }

    if(!ASSERT(poll_buff_size == UPOLL_UNIT_SIZE))
    {   DIECAT("%s", "Unexpected size of poll_buff array");
    }

    if(!ASSERT(poll_fds_len == poll_events_len && poll_fds_len == poll_revents_len && poll_fds_len == poll_buff_len))
    {   DIECAT("%s", "Inconsistent poll array sizes");
    }

    garray_i i;

    const i64 WAIT_TIME_MS_CONVERSION = 1000;
    /* write can fail very rarely but sometimes still occur, so 10s is long enough wakeup time */
    float seconds = 10;

    i64 wait_time_ms = WAIT_TIME_MS_CONVERSION * seconds;

    has_pipe = 0;
    /* 200ms is slow neough that this spurious wakeup shouldnt matter too too much */
    if(!has_pipe)
    {
        seconds = .2f;   
        wait_time_ms = WAIT_TIME_MS_CONVERSION * seconds;
    }

    do
    {
        errno = 0;

        status = upoll_poll_mult(poll_fds, poll_events, poll_revents, poll_buff, poll_fds_len, wait_time_ms);

        if(status < 0)
        {
            int err = errno;

            if(!ASSERT(err != EFAULT))
            {   DIECAT("%s", "upoll_poll_mult failed with EFAULT");
            }

            if(!ASSERT(err != EINVAL))
            {   DIECAT("%s", "upoll_poll_mult failed with EINVAL");
            }

            if(err == ENOMEM)
            {
                WMPollCallExit();
                goto EXIT;
            }

            DIECAT("upoll_poll_mult failed with errno %d", err);
        }

        if(wm_poll_exit)
        {   goto EXIT;
        }

        /*
         * upoll converts EINTR into 0.
         */
        if(status == 0)
        {   continue;
        }

        int *fds = poll_fds;
        int *revents = poll_revents;

        for(i = 0; i < poll_fds_len; ++i)
        {
            /*
             * poll() may return POLLERR, POLLHUP, and POLLNVAL
             * even if they weren't requested in events[].
             */
            if(revents[i])
            {
                void (**callback_tmp)(int fd, int revents, Generic arg) = GArrayAt(&wm_poll_callbacks, i);

                if(!ASSERT(callback_tmp != NULL))
                {   DIECAT("%s", "Callback is NULL");
                }

                void (*callback)(int fd, int revents, Generic arg) = *callback_tmp;

                void *arg_tmp = GArrayAt(&wm_poll_args, i);

                if(!ASSERT(arg_tmp != NULL))
                {   DIECAT("%s", "Arg is NULL");
                }

                Generic arg = *(Generic *)arg_tmp;

                if(!ASSERT(callback != NULL))
                {   DIECAT("%s", "Callback is NULL");
                }

                /*
                 * Copy these before callback() because callback()
                 * may call WMPollAddFD(), reallocating the GArrays.
                 */
                int fd = fds[i];
                int fd_revents = revents[i];

                callback(fd, fd_revents, arg);

                if(wm_poll_exit)
                {   goto EXIT;
                }

                /*
                 * Reacquire backing pointers in case callback()
                 * caused a GArray realloc.
                 */
                status = GArrayGetArray(&wm_poll_fds, &poll_fds, &poll_fds_len, &unused, &poll_fds_size);

                if(!ASSERT(status == EXIT_SUCCESS))
                {   DIECAT("%s", "Failed to get array from wm_poll_fds");
                }

                status = GArrayGetArray(&wm_poll_events, &poll_events, &poll_events_len, &unused, &poll_events_size);

                if(!ASSERT(status == EXIT_SUCCESS))
                {   DIECAT("%s", "Failed to get array from wm_poll_events");
                }

                status = GArrayGetArray(&wm_poll_revents, &poll_revents, &poll_revents_len, &unused, &poll_revents_size);

                if(!ASSERT(status == EXIT_SUCCESS))
                {   DIECAT("%s", "Failed to get array from wm_poll_revents");
                }

                status = GArrayGetArray(&wm_poll_buff, &poll_buff, &poll_buff_len, &unused, &poll_buff_size);

                if(!ASSERT(status == EXIT_SUCCESS))
                {   DIECAT("%s", "Failed to get array from wm_poll_buff");
                }

                if(!ASSERT(poll_fds_len == poll_events_len &&
                           poll_fds_len == poll_revents_len &&
                           poll_fds_len == poll_buff_len))
                {   DIECAT("%s", "Inconsistent poll array sizes");
                }

                fds = poll_fds;
                revents = poll_revents;
            }
        }
    }
    while(!wm_poll_exit);

EXIT:
    return EXIT_FAILURE;
}

void
WMPollWakeup(void)
{
    if(!has_pipe)
    {   return;
    }

    char c = 1;

    if(write(wake_pipe[PIPE_WRITE], &c, 1) == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {   return;
        }

        /* shit */
        DebugError("Failed to write to wake pipe, were fucked");
    }
}

void
WMPollCallExit(void)
{   wm_poll_exit = 1;
}

void 
WMPollDestroy(void)
{
    GArrayWipe(&wm_poll_fds);
    GArrayWipe(&wm_poll_events);
    GArrayWipe(&wm_poll_revents);
    GArrayWipe(&wm_poll_callbacks);
    GArrayWipe(&wm_poll_args);
    GArrayWipe(&wm_poll_buff);

    if(has_pipe)
    {
        close(wake_pipe[PIPE_READ]);
        close(wake_pipe[PIPE_WRITE]);

        /* set to invalid file descriptors */
        wake_pipe[PIPE_READ] = -1;
        wake_pipe[PIPE_WRITE] = -1;
    }

    wm_poll_exit = 0;
    has_pipe = 0;
}
