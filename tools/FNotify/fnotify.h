#ifndef __F__NOTIFY__H__
#define __F__NOTIFY__H__


#ifdef __cplusplus
extern "C" {
#endif

#if defined(__linux__)
#include <sys/inotify.h>
#else
/* cant read config otherwise */
#warning Failed to load inotify for platform 
#endif

#include <pthread.h>


enum
#if defined(__GNUC__) || defined(__clang__)
__attribute__((flag_enum))
#endif

FNotifyFlags
{
    FNotifyFileAccess           = 1 << 0,
    FNotifyFileMetaDataModify   = 1 << 1,
    FNotifyClosedWrite          = 1 << 2,
    FNotifyClosedRead           = 1 << 3,
    FNotifyFileCreate           = 1 << 4,
    FNotifyDirCreate            = 1 << 5,
    FNotifyFileMovedSelf        = 1 << 6,
    FNotifyFileMovedTo          = 1 << 7,
    FNotifyFileMovedFrom        = 1 << 8,
    FNotifyFileDeletedSelf      = 1 << 9,
    FNotifyFileDeleted          = 1 << 10,
    FNotifyFileModify           = 1 << 11,
    FNotifyFileOpened           = 1 << 12,

    /* ensure compiler gets minimum flag correctly */
    FNotifySpareBit             = 1 << 13,
};

typedef struct FNotify FNotify;
typedef struct FNotifyEvent FNotifyEvent;

struct
FNotify
{
    int inotify_fd;
    int inotify_wd;
    int inotify_mask;
    uint8_t pad[4];
    pthread_mutex_t mutex;
};

struct
FNotifyEvent
{
    enum FNotifyFlags mask;
};


/* Allocate a fnotify structure.
 *
 * RETURN: FNotify * on Success.
 * RETURN: NULL on Failure.
 */
FNotify *
FNotifyCreate(
        char *FILE_NAME,
        enum FNotifyFlags flags
        );

/* Fill a fnotify structure.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
FNotifyCreateFilled(
        FNotify *fill_return,
        char *FILE_NAME,
        enum FNotifyFlags flags
        );

/* Waits for a fnotify event to occur, and fills event_fill data, respecting events_len_fill_max.
 *
 * NOTE: return of event_count MAY return 0.
 *
 * RETURN: event_count, number of events filled.
 * RETURN: -1 on Failure.
 */
int
FNotifyWaitForEvent(
        FNotify *notify,
        FNotifyEvent *event_fill,
        uint32_t events_len_fill_max
        );

/* Polls for a event, and fills it if availabe.
 *
 * NOTE: return of event_count MAY return 0.
 *
 * RETURN: event_count, number of events filled.
 * RETURN: -1 on Poll Error.
 * RETURN: -2 on Timeout.
 */
int 
FNotifyPollForEvent(
        FNotify *notify,
        FNotifyEvent *event_fill,
        uint32_t events_len_fill_max,
        int timeout_milliseconds,
        int *error
        );

/* Destroys Fnotify data.
 *
 * NOTE: This does not free the notify pointer if it was allocated using FNotifyCreate();
 *
 */
void
FNotifyDestroy(
        FNotify *notify
        );


#ifdef __cplusplus
}
#endif

#endif
