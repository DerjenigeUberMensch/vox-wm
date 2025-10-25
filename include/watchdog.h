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
    uint8_t pad0[1];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    time_t last_alive;
};

/* Starts up the WatchDog Process.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WatchDogStart(
    int argc,
    char **argv
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

/* For program sanity, this should be called at atleast every second, though this can be extented to ever 2 seconds.
 *
 * NOTE: This function does not block, unless past the watchdog threshhold at which point your application will restart.
 *
 */
void
WatchDogPingAlive(
    void
);



#endif
