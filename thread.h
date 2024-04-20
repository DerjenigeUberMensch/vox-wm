#ifndef THREADING_H_
#define THREADING_H_

#include <pthread.h>
#include <stdint.h>


typedef struct Thread Thread;
typedef pthread_t Handle;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;
typedef pthread_spinlock_t MutexQuick;





struct Thread
{
    Handle id;                  /* Thread ID        */
    Mutex *mutex;               /* Thread Mutex     */
    Mutex *_mutex;              /* PRIVATE          */
    Cond *cond;                 /* Thread Condition */
    MutexQuick *mutexq;         /* Thread SpinLock  */
    MutexQuick *_mutexq;        /* PRIVATE          */
    void *data;                 /* Thread Arg       */
    void (*func ) (void *data); /* Thread Function  */
    uint32_t flags;             /* Flags for Thread */
    uint8_t pad0[4];
};

/* Creates a thread with the optional parametor void function(void *data) and optional void *data;
 * 
 *
 * Default thread behaviour is to run forever.
 * EX: 
 *     while(1)
 *     {
 *         yourfunction();
 *     } 
 *
 *
 * Caller must call ThreadExit(); or ThreadJoin(); or ThreadKill();
 * When done using data.
 *
 * RETURN: Thread * on Success.
 * RETURN: NULL on Failure.
*/
Thread *
ThreadCreate(
        void (*func) (void *data), void *data
        );

/* TODO
 */
Thread *
ThreadGetSelf(
        void
        );

/* Detach the thread specified from the main process.
 *
 * thread:          Thread *        The thread you want to make call exit();
 *                  NULL            NaN.
 */
void
ThreadDetach(
        Thread *thread
        );

/* Makes the thread exit after the last sucessfull function call of the thread.
 * Thread must be thought as unrecheable unless the function looping is a infinite loop.
 *
 *
 * thread:          Thread *        The thread you want to make call exit();
 *                  NULL            Exits the current thread.
 * 
 *
 * NOTE: All data relating to thread is freed and must not be accessed.
 * NOTE: This function does NOT block.
 */
void 
ThreadExit(
        Thread *thread
        );
/* Sends the signal SIGTERM to the provided thread and frees all resulting thread data.
 * Caller must call pthread_exit() if possible, 
 * else default protocol is to die after the last successfull function call.
 * 
 * thread:          Thread *        The thread you want to kill
 *                  NULL            Kill the current thread.
 *
 * NOTE: This function does NOT block.
 */
void
ThreadKill(
        Thread *thread
        );

/* Joins the thread with the main thread, blocking till its done.
 *
 * thread:          Thread *        The thread you want to join
 *                  NULL            NaN
 *
 * NOTE: All data relating to thread is freed and must not be accessed.
 * NOTE: This function DOES block.    
 */
void 
ThreadJoin(
        Thread *thread
        );
/* Mutex locks a thread preventing other threads from accessing it until the mutex is unlocked.
 * This should be used for most slower operations, functions, memory allocations, etc...
 * 
 * Must be unlocked using ThreadUnlock();
 *
 * thread:          Thread *        The thread you want to Lock 
 *                  NULL            Lock the current thread.
 *
 * RETURN: 1 on Success.
 * RETURN: 0 on Failure. (probably already locked)
 */
uint8_t
ThreadLock(
        Thread *thread
        );
/* Non blocking, instead a while (1) { No Operation } loop occurs.
 * This prevents the scheduler from sleeping the thread which causes a context switch and is slow.
 * This should only be used for very light small operations, setting a variable(s), incrementing a counter, etc...
 * 
 * Must be unlocked using ThreadUnlockQuick();
 *
 * thread:          Thread *        The thread you want to Quick Lock
 *                  NULL            Quick Lock the current thread.
 *
 * RETURN: 1 on Success.
 * RETURN: 0 on Failure. (probably already locked)
 */
uint8_t
ThreadLockQuick(
        Thread *thread
        );
/* Unlocks a previously locked thread.
 *
 * thread:          Thread *        The thread you want to UnLock
 *                  NULL            UnLock the current thread.
 */
void 
ThreadUnlock(
        Thread *thread
        );
/* Unlocks a previously quicklocked thread.
 *
 * thread:          Thread *        The thread you want to Quick UnLock
 *                  NULL            Quick UnLock the current thread.
 */
void
ThreadUnlockQuick(
        Thread *thread
        );

/* Signals the threads condition to awake from its previously waiting state.
 *
 * thread:          Thread *        The thread you want to Signal
 *                  NULL            Signal the current thread.
 */
void 
ThreadSignal(
        Thread *thread
        );
/* Makes a thread wait for a condition to be done.
 * cond:                        Your own condition or &(thread->cond)
 *
 * thread:          Thread *        The thread you want to wait till a Condition. 
 *                  NULL            Sleep the current thread till a Condition is met.
 */
void 
ThreadCondWait(
        Thread *thread, 
        Cond *cond
        );

/* Not adviced to do so unless initializing.
 * Changes the function that is called in the thread.
 * Should be relativily safe and take effect on the next function call. 
 *
 * thread:          Thread *        The thread you want to change the function of.
 *                  NULL            NaN.
 */
void
ThreadChangeCaller(
        Thread *thread,
        void (*func)(void *data)
        );
/* Not adviced to do so unless initializing
 * Changes the data that is passed to the calling function, in the thread.
 * Should be relativily safe and take effect on the next function call. 
 *
 * thread:          Thread *        The thread you want to change the function of.
 *                  NULL            NaN.
 */
void
ThreadChangeCallerData(
        Thread *thread,
        void *data
        );
/* Changes the function and data to caller.
 * Should be safe and take effect on the next function call. 
 * 
 * thread:          Thread *        The thread you want to change the function of.
 *                  NULL            NaN.
 */
void
ThreadChangeFunction(
    Thread *thread,
    void (*func) (void *data),
    void *data
    );
    
#endif
