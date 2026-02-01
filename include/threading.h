#include "TPromises/tpromise.h"
#include "util.h"
#include "safebool.h"


typedef struct ThreadWorkItem ThreadWorkItem;
typedef struct ThreadWorkItem2 ThreadWorkItem2;

struct
ThreadWorkItem
{
    Generic arg;
    TPromise *promise;
    void (*func)(Generic *);
};

struct
ThreadWorkItem2
{
    Generic arg1;
    Generic arg2;
    TPromise *promise;
    void (*func)(Generic *);
};


/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int InitThreading(void);
int ThreadingAddWork(void (*callback)(Generic *arg), Generic *arg, TPromise *optional_tpromise_to_use);
void ThreadingWaitAll(void);
void ThreadingDestroy(void);
void ThreadingPause(void);
void ThreadingResume(void);
int ThreadingGetNumWorking(void);
bool ThreadingUsesThreads(void);
