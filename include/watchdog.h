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
    uint8_t die;
    uint8_t restart_timer;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    void (*callback)(void);
    struct timespec last_alive;
};

/* Starts up the WatchDog Process.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WatchDogStart(
    int argc,
    char **argv,
    void (*callback_on_stall)(void)
);

/* Requests the watchdog to terminate, and allow for application to terminate gracefully.
 *
 * NOTE: This function (may) block, until watchdog is ready to exit.
 * NOTE: This should ONLY be called when the main application is about to exit AFTER cleanup.
 *
 * This function does not return a value.
 */
void
WatchDogRequestExit(
    void
);

/* This resets the hard timer in the watchdog process.
 *
 * NOTE: This function does not block.
 *
 */
void
WatchDogRespond(
    void
);

#endif
