#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <poll.h>

#include <string.h>
#include "fnotify.h"


static enum FNotifyFlags 
FNOTIFY_GET_INOTIFY_FLAGS_FNOTIFY(uint32_t mask)
{
    uint32_t ret = 0;

    if(mask & IN_ACCESS)
    {   ret |= FNotifyFileAccess;
    }
    if(mask & IN_ATTRIB)
    {   ret |= FNotifyFileMetaDataModify;
    }
    if(mask & IN_CLOSE_WRITE)
    {   ret |= FNotifyClosedWrite;
    }
    if(mask & IN_CLOSE_NOWRITE)
    {   ret |= FNotifyClosedRead;
    }
    if(mask & IN_CREATE)
    {   ret |= FNotifyFileCreate|FNotifyDirCreate;
    }
    if(mask & IN_MOVE)
    {   ret |= FNotifyFileMoved;
    }
    if(mask & IN_DELETE)
    {   ret |= FNotifyFileDeleted;
    }
    if(mask & IN_MODIFY)
    {   ret |= FNotifyFileModify;
    }
    if(mask & IN_OPEN)
    {   ret |= FNotifyFileOpened;
    }

    return ret;
}

uint32_t
FNotifyGetCorrectFlagsAdd(
        enum FNotifyFlags flags
        )
{
    uint32_t ret = 0;
    uint32_t flag = 0;

    flag = IN_ACCESS;

    if(flags & FNotifyFileAccess)
    {   ret |= flag;
    }

    flag = IN_ATTRIB;

    if(flags & FNotifyFileMetaDataModify)
    {   ret |= flag;
    }

    flag = IN_CLOSE_WRITE;

    if(flags & FNotifyClosedWrite)
    {   ret |= flag;
    }

    flag = IN_CLOSE_NOWRITE;

    if(flags & FNotifyClosedRead)
    {   ret |= flag;
    }

    flag = IN_CREATE;

    if(flags & FNotifyFileCreate)
    {   ret |= flag;
    }

    flag = IN_CREATE;

    if(flags & FNotifyDirCreate)
    {   ret |= flag;
    }

    flag = IN_MOVE;

    if(flags & FNotifyFileMoved)
    {   ret |= flag;
    }

    flag = IN_DELETE;

    if(flags & FNotifyFileDeleted)
    {   ret |= flag;
    }

    flag = IN_MODIFY;

    if(flags & FNotifyFileModify)
    {   ret |= flag;
    }

    flag = IN_OPEN;

    if(flags & FNotifyFileOpened)
    {   ret |= flag;
    }

    return ret;
}

FNotify *
FNotifyCreate(
        const char *const FILE_NAME,
        enum FNotifyFlags flags
        )
{
    FNotify *ret = malloc(sizeof(FNotify));
    if(ret)
    {
        int status = FNotifyCreateFilled(ret, FILE_NAME, flags);
        if(status == EXIT_FAILURE)
        {   
            free(ret);
            ret = NULL;
        }
    }
    return ret;
}

int
FNotifyCreateFilled(
        FNotify *fill_return,
        const char *const FILE_NAME,
        enum FNotifyFlags flags
        )
{
    int status;
    
    if(!fill_return || !FILE_NAME || !flags)
    {   return EXIT_FAILURE;
    }

    /* intialize mutex */
    const pthread_mutex_t intializer = PTHREAD_MUTEX_INITIALIZER;

    fill_return->mutex = intializer;

    enum
    {   INOTIFY_FAILURE = -1,
    };

    status = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);

    if(status == INOTIFY_FAILURE)
    {   goto FAILURE;
    }

    uint32_t mask = FNotifyGetCorrectFlagsAdd(flags);

    int fd;

    fd = inotify_add_watch(fill_return->inotify_fd, FILE_NAME, mask);

    enum { INOTIFY_ERR = -1 };

    if(fd != INOTIFY_ERR)
    {
        fill_return->inotify_wd = fd;
        fill_return->inotify_mask = mask;
    }
    else
    {   
        close(fill_return->inotify_fd);
        goto FAILURE;
    }

    fill_return->inotify_fd = status;

    return EXIT_SUCCESS;
    /* failure cleanup up resources */
FAILURE:
    return EXIT_FAILURE;
}

int 
FNotifyPollForEvent(
        FNotify *notify,
        FNotifyEvent *event_fill,
        uint32_t events_len_fill_max,
        int timeout_milliseconds,
        int *error
        )
{
    enum { TIMEOUT_ERROR = -2 };
    enum { POLL_ERROR = -1 };

    if(!notify)
    {   return POLL_ERROR;
    }

    /* timeout in milliseconds seconds = TIMEOUT/1000 */
    enum { TIMEOUT = 50 };

    struct pollfd fd = {0};

    fd.fd = notify->inotify_fd;
    fd.events = POLLIN;

    int timeout = timeout_milliseconds == -1 ? TIMEOUT : timeout_milliseconds;

    int status = poll(&fd, 1, timeout);

    /* some poll error */
    if(status < 0)
    {   return POLL_ERROR;
    }
    /* timed out */
    if(status == 0)
    {   return TIMEOUT_ERROR;
    }

    if(fd.revents & POLLIN)
    {   return FNotifyWaitForEvent(notify, event_fill, events_len_fill_max);
    }

    if(fd.revents & (POLLERR | POLLHUP | POLLNVAL))
    {   return POLL_ERROR;
    }

    return TIMEOUT;
}

int
FNotifyWaitForEvent(
        FNotify *notify,
        FNotifyEvent *event_fill,
        uint32_t events_len_fill_max
        )
{
    if(events_len_fill_max == 0)
    {   return -1;
    }

    /* truncated so size doesnt matter, but just use a stadnard. TODO FIX later if to short. */
    enum { MAX_FILENAME = 1024 };

    char buff[MAX_FILENAME + sizeof(struct inotify_event)];
    char *offset;
    struct inotify_event *event;
    int64_t event_count = 0;

    while(1)
    {
        ssize_t bytes_read = read(notify->inotify_fd, buff, sizeof(buff));

        if(bytes_read == 0)
        {   return event_count;
        }

        if(bytes_read < 0)
        {   
            if(event_count == 0)
            {   return -1;
            }

            return event_count;
        }

        for(offset = buff; offset < buff + bytes_read;)
        {
            event = (void *)offset;

            event_fill[event_count].mask = FNOTIFY_GET_INOTIFY_FLAGS_FNOTIFY(event->mask);

            ++event_count;

            if(event_count == events_len_fill_max)
            {   break;
            }

            offset += sizeof(struct inotify_event) + event->len;
        }
    }
}

void
FNotifyDestroy(
        FNotify *notify
        )
{
    if(!notify)
    {   return;
    }

    inotify_rm_watch(notify->inotify_fd, notify->inotify_wd);

    close(notify->inotify_fd);
}

