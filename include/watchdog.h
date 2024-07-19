#ifndef _WATCHDOG_H_42
#define _WATCHDOG_H_42

#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "XCB-TRL/xcb_trl.h"

/* Not sure which looks better WatchDog or Watchdog */

typedef struct WatchDog WatchDog;

struct
WatchDog
{
    int running;
    pid_t child;
    uint8_t restart;
    uint8_t fail_count;
    uint8_t pad0[2];
    /* Maybe use poll?, For now just use these to sleep */
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/* Starts up the WatchDog Process.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WatchDogStart(
        void
        );
/* This is the main WatchDog Event loop.
 */
int
WatchDogRun(
        void
        );







#endif
