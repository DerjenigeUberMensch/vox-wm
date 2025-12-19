#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include "watchdog.h"
#include "util.h"

static WatchDog *watchdog = NULL;

int WatchDogRun(int argc, char **argv);

int
WatchDogInit(
    void
    )
{
    pthread_mutexattr_t attrmutex;
    pthread_condattr_t attrcond;
    int status;

    watchdog = mmap(NULL, sizeof(WatchDog), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    if(!watchdog)
    {   return EXIT_FAILURE;
    }

    memset(watchdog, 0, sizeof(WatchDog));

    status = pthread_mutexattr_init(&attrmutex);

    if(status)
    {   goto UNMAP;
    }

    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);

    status = pthread_condattr_init(&attrcond);
    
    if(status)
    {   goto UNMUTXATTR;
    }

    pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);

    status = pthread_mutex_init(&watchdog->mutex, &attrmutex);

    if(status)
    {   goto UNCONDATTR;
    }

    status = pthread_cond_init(&watchdog->cond, &attrcond);

    if(status)
    { 	goto UNMUTEX;
    }


    return EXIT_SUCCESS;
UNMUTEX:
    pthread_mutex_destroy(&watchdog->mutex);
UNCONDATTR:
    pthread_condattr_destroy(&attrcond);
UNMUTXATTR:
    pthread_mutexattr_destroy(&attrmutex);
UNMAP:
    munmap(watchdog, sizeof(*watchdog));
    return EXIT_FAILURE;
}

void
WatchDogDestroy(
    void
    )
{
    pthread_cond_destroy(&watchdog->cond);
    pthread_mutex_destroy(&watchdog->mutex);
    munmap(watchdog, sizeof(*watchdog));
    watchdog = NULL;
}

int
WatchDogStart(
    int argc,
    char **argv
    )
{
    pid_t pid;

    /* watchdog failed to start */
    if(WatchDogInit() != EXIT_SUCCESS)
    {   return EXIT_FAILURE;
    }

    pid = fork();

    switch(pid)
    {
        /* watchdog failed to start */
        case -1:
            return EXIT_FAILURE;
        case 0:
            WatchDogRun(argc, argv);
	    exit(EXIT_SUCCESS);
            break;
        default:
            pthread_mutex_lock(&watchdog->mutex);
            watchdog->child = getpid();
            watchdog->last_alive = time(NULL);
            pthread_mutex_unlock(&watchdog->mutex);
            Debug0("WatchDog Succesfully Started!");
    }

    return EXIT_SUCCESS;
}

void
WatchDogRequestExit(
    void
)
{
    if(!watchdog)
    {   return;
    }

    pthread_mutex_lock(&watchdog->mutex);

    pthread_cond_broadcast(&watchdog->cond);

    watchdog->die = 1;

    while(watchdog->running)
    {   pthread_cond_wait(&watchdog->cond, &watchdog->mutex);
    }

    pthread_mutex_unlock(&watchdog->mutex);

    /* Mutex is never held after terminating last mutex hold after watchdog->running is set to false. */
    WatchDogDestroy();
}

void
WatchDogPingAlive(
    void
)
{
    if(!watchdog)
    {   return;
    }
    pthread_mutex_lock(&watchdog->mutex);

    watchdog->last_alive = time(NULL);

    pthread_mutex_unlock(&watchdog->mutex);
}

int
WatchDogRun(
    int argc,
    char **argv
)
{
    int status;

    pthread_mutex_lock(&watchdog->mutex);
    while(!watchdog->child)
    {   pthread_cond_wait(&watchdog->cond, &watchdog->mutex);
    }

    watchdog->running = 1;

    /* due to speed, and blah blah blah we dont want this to hang or whatever... */
    const unsigned int SECONDS = 1;
    const unsigned int MAX_TIMEOUT = 7;
    struct timespec PING_TIME;

    bool child_dead = false;

    while(watchdog->running && !watchdog->die)
    {
	status = clock_gettime(CLOCK_REALTIME, &PING_TIME);
	PING_TIME.tv_sec += SECONDS;
        /* wait... */
        status = pthread_cond_timedwait(&watchdog->cond, &watchdog->mutex, &PING_TIME);

	/* ok were kinda fucked here... 
	 * This is here cause the above immediatly fails since the time is in the past....
	 */
	if(status == -1)
	{   sleep(SECONDS);
	}

        /* is the process alive? */
        if(kill(watchdog->child, 0) != EXIT_SUCCESS)
        {
            if(errno == ESRCH)
            {   
                watchdog->running = 0;
                /* Just in case process magically spawns in. */
                watchdog->restart = 1;
                child_dead = true;
                break;
            }
        }

        time_t now = time(NULL);
	Debug("%ld", now - watchdog->last_alive);

        /* 7 seconds is more than enough. */
        if (now - watchdog->last_alive > MAX_TIMEOUT)
        {   
            kill(watchdog->child, SIGKILL);
	    #ifdef DEBUG
		kill(watchdog->child, SIGABRT);
	    #else
		kill(watchdog->child, SIGKILL);
	    #endif
            watchdog->running = 0;
            /* Process might hang for a undefined length of time so this just forces a restart regardless. */
            watchdog->restart = 1;
            child_dead = true;

            if (waitpid(watchdog->child, &status, 0) > 0) 
            {   
                int ERROR_STATUS = -1;

                if(status == ERROR_STATUS)
                {   
                    if(errno == ECHILD)
                    {   
                        Debug0("Child died, before killing.");
                        break;
                    }

                    /* we got interrupted */
                    if(errno == EINTR)
                    {
                        /* if its still alive, abort this shouldnt be happening. */
                        if(kill(watchdog->child, 0) == EXIT_SUCCESS)
                        {   kill(watchdog->child, SIGABRT);
                        }
                        break;
                    }
                }
            }
            break;
        }
    }

    watchdog->running = 0;
    pthread_cond_broadcast(&watchdog->cond);

    /* This line is purposely left here commented.
     * Due to cleanup code, we do not want the watchdog to ever need to UNLOCK.
     */
    /* pthread_mutex_unlock(&watchdog->mutex); */
    /* WM is succesfully cleaned up, confirmation is simply needed to exit process. */
    if(watchdog->die)
    {   
        pthread_mutex_unlock(&watchdog->mutex);
	_exit(EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }

    if(kill(watchdog->child, 0) != EXIT_SUCCESS)
    {   
        if(errno == ESRCH)
        {   child_dead = true;
        }
    }

    if(likely(watchdog->restart))
    {
        WatchDogDestroy();

        /* This should be unreachable but could maybe happen. */
        if(unlikely(!child_dead))
        {   	
	    #ifdef DEBUG
		kill(watchdog->child, SIGABRT);
	    #else
		kill(watchdog->child, SIGKILL);
	    #endif
        }

        Debug0("Watchdog Restarting Process...");
        #ifdef __linux__
            char *path = "/proc/self/exe";
            execv(path, argv);
            perror("execv failed.");
        #endif

        execvp(argv[0], argv);
        perror("execvp failed.");
    }

    if(likely(child_dead))
    {   
        Debug0("Exiting, child dead?? FIXME.");
	_exit(EXIT_SUCCESS);
    }


    ASSERT(false);

    return EXIT_FAILURE;
}
