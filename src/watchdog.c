#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

#include "watchdog.h"
#include "util.h"

WatchDog *watchdog = NULL;

static void
sigany(
        int signo
      )
{
    /* These are cases where we dont explicitly want any action taken, unless otherwise stated 
     * AKA cases where we dont want the system to intervene.
     */
    (void)signo;
}

static void
sigquit(
        int signo
       )
{
    Debug0("Aborting...");
    abort();
}

static void
sigterm(
        int signo
       )
{
}

static void
sigsegv(
        int signo
      )
{
    /* No, we arent handling our own segmentation fault, that should never happen, unless the user overwrites memory manually.
     * But we still need to make sure the (XEH) doesnt crash completely and atleast let the user close their stuff, before telling them.
     * "The current (XEH) is not safe to use please restart the (XEH), preferably restart the XServer."
     */
    Debug0("Sigsev");
    watchdog->restart = 1;
    abort();
}

static void
WatchDogInstallSignal(
        void (*func)(int signo),
        int signo
        )
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = func;
    sigaction(signo, &sa, NULL);
}

static void
WatchDogInstallSignals(
        void
        )
{
    WatchDogInstallSignal(sigany, SIGHUP);
    WatchDogInstallSignal(sigany, SIGINT);
    WatchDogInstallSignal(sigsegv, SIGSEGV);
    WatchDogInstallSignal(sigquit, SIGQUIT);
    WatchDogInstallSignal(sigquit, SIGFPE);
}

static int
WatchDogSetSignals(
        pid_t pid
        )
{
    int status = 0;
    waitpid(pid, &status, WNOHANG|WUNTRACED);

    if(status == -1)
    {   
        Debug0("Waitpid failed");
        return 1;
    }

    status = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if(status == -1)
    {   
        Debug0("Ptrace failed to attach,");
        return 1;
    }
    /* wait for main procces to hold (SIGSTOP) */
    waitpid(pid, &status, 0);

    if(status == -1)
    {   
        Debug0("Waitpid failed");
        return 1;
    }
    WatchDogInstallSignals();
    status = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    /* Should not happen */
    if(status == -1)
    {   
        Debug0("Ptraced failed to dettach");
        abort();
    }
    return 0;
}

void
WatchDogInit(
        pid_t pid
        )
{
    WatchDogSetSignals(pid);
    watchdog->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    watchdog->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
}

int
WatchDogStart(
        void
        )
{
    watchdog = mmap(NULL, sizeof(WatchDog), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(!watchdog)
    {   return EXIT_FAILURE;
    }
    memset(watchdog, 0, sizeof(WatchDog));
    pid_t pid = fork();
    switch(pid)
    {
        /* watchdog failed to start */
        case -1:
            return EXIT_FAILURE;
        case 0:
            Debug0("Watchdog Child, valid");
            return WatchDogRun();
        default:
            WatchDogInit(pid);
            watchdog->child = pid;
            Debug0("Watch Dog Succesfully Started!");
    }
    return EXIT_SUCCESS;
}

int
WatchDogRun(
        void
        )
{
    /* Wait for watchdog data */
    while(!watchdog->child)
    {   usleep(100);
    }
    const unsigned int SECONDS = 10;
    const struct timespec _time = 
    {
        .tv_sec = SECONDS,
        .tv_nsec = 0
    };
    watchdog->running = 1;
    while(watchdog->running && !watchdog->restart)
    {
        pthread_mutex_lock(&watchdog->mutex);
        pthread_cond_timedwait(&watchdog->cond, &watchdog->mutex, &_time);
        pthread_mutex_unlock(&watchdog->mutex);
    }
    if(watchdog->restart)
    {   Debug0("restarting");
    }
    else
    {   exit(0);
    }
    signal(SIGSEGV, NULL);
    usleep(100);
    munmap(watchdog, sizeof(WatchDog));
    return 10;
}
