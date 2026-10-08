#ifndef __WM__POLL__H__
#define __WM__POLL__H__

#include "util.h"

/* Intialize the poll system.
 *
 * The poll system is fatal if we cannot initialize it properly.
 * It is used to monitor multiple file descriptors for I/O events.
 * We have a preset MAX guranteed about using NUM_OF_POLL_FDS.
 * If we past that then we are allowed to call ABORT() so if you are using this
 * system, make sure you don't exceed the limit.
 *
 * This function does not return a value.
 */
void WMPollInit(int NUM_OF_POLL_FDS);
/* Add a file descriptor to the poll system.
 *
 * events      int      
 *
 * This function does not return a value.
 */
void WMPollAddFD(int fd, int events, void (*callback)(int fd, int events, Generic arg), Generic arg);
/* Run the poll system..
 *
 * RETURN: If the EXIT_SUCCESS status is returned, the poll system can be re-run again and should generally be re run again.
 * RETURN: If the EXIT_FAILURE status is returned, the poll system has failed and should not be re-run.
 */
int WMPollRun(void);

/* Wake up the poll system.
 *
 * NOTE: This functions is thread safe probably...
 *
 * This function does not return a value.
 */
void WMPollWakeup(void);
/* Call this function to exit the poll system.
 * 
 * NOTE: This can only be called inside a callback function.
 *
 * This function does not return a value.
 */
void WMPollCallExit(void);
/* 
 * Destroy the poll system.
 */
void WMPollDestroy(void);


#endif