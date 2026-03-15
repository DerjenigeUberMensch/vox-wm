#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#ifdef __linux__
#include <sys/prctl.h>
#endif

#include "watchdog.h"
#include "util.h"

static WatchDog *watchdog = NULL;

int WatchDogRun(int argc, char **argv);

int
WatchDogInit(
    void (*callback_on_stall)(void)
    )
{
    pthread_mutexattr_t attrmutex;
    pthread_condattr_t attrcond;
    int status;

    if(!ASSERT(callback_on_stall))
    {   return EXIT_FAILURE;
    }

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

    /* cond doesnt support CLOCK_MONOTONIC_COARSE cor some reason -\(!-!)/- */
    pthread_condattr_setclock(&attrcond, CLOCK_MONOTONIC);

    status = pthread_mutex_init(&watchdog->mutex, &attrmutex);

    if(status)
    {   goto UNCONDATTR;
    }

    status = pthread_cond_init(&watchdog->cond, &attrcond);

    if(status)
    { 	goto UNMUTEX;
    }

    watchdog->callback = callback_on_stall;

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
    char **argv,
    void (*callback_on_stall)(void)
    )
{
    pid_t pid;
    struct rlimit rl;
    int fd;
    int status;

    /* watchdog failed to start */
    if(WatchDogInit(callback_on_stall) != EXIT_SUCCESS)
    {   return EXIT_FAILURE;
    }

    pid = fork();

    switch(pid)
    {
        /* watchdog failed to start */
        case -1:
            return EXIT_FAILURE;
        case 0:
            if(setsid() < 0)
            {   exit(EXIT_FAILURE);
            }

            status = chdir("/");

            if(status == -1)
            {   fprintf(stderr, "Failed to unmount daemon.");
            }

            umask(0);

            getrlimit(RLIMIT_NOFILE, &rl);

            /* stdin, stdout, stderr */
            enum { IMPORTANT_FILE_DESCRIPTORS = 2 };

            for(fd = 0; fd < rl.rlim_max; ++fd)
            {   
                if(fd > IMPORTANT_FILE_DESCRIPTORS)
                {   close(fd);
                }
            }

            #ifdef __linux__
                prctl(PR_SET_NAME, "vox-wm-watchdog", 0, 0, 0);
            #endif
            WatchDogRun(argc, argv);
	        exit(EXIT_SUCCESS);
            break;
        default:
            pthread_mutex_lock(&watchdog->mutex);
            watchdog->child = getpid();

            #if __linux__
                clock_gettime(CLOCK_MONOTONIC_COARSE, &watchdog->last_alive);
            #else
                clock_gettime(CLOCK_MONOTONIC, &watchdog->last_alive);
            #endif

            pthread_mutex_unlock(&watchdog->mutex);
            Debug0("WatchDog Succesfully Started!");
            break;
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
WatchDogRespond(
    void
)
{
    if(!watchdog)
    {   return;
    }

    pthread_mutex_lock(&watchdog->mutex);
    watchdog->restart_timer = 1;
    Debug0("shi");
    pthread_mutex_unlock(&watchdog->mutex);
}

static double
timespecdiff(struct timespec *start, struct timespec *end)
{
    double sec = (double)(end->tv_sec - start->tv_sec);
    double nsec = (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return sec + nsec;
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
    const unsigned int MAX_TIMEOUT = 15;
    struct timespec PING_TIME;

    bool child_dead = false;

    while(watchdog->running && !watchdog->die)
    {
        #if __linux__
            clock_gettime(CLOCK_MONOTONIC_COARSE, &PING_TIME);
        #else
            clock_gettime(CLOCK_MONOTONIC, &PING_TIME);
        #endif

        PING_TIME.tv_sec += SECONDS;
        /* wait... */
        status = pthread_cond_timedwait(&watchdog->cond, &watchdog->mutex, &PING_TIME);

        /* ok were kinda fucked here... 
         * This is here cause the above immediatly fails since the time is in the past....
         */
        if(status == -1)
        {   
            pthread_mutex_unlock(&watchdog->mutex);
            sleep(SECONDS);
            pthread_mutex_lock(&watchdog->mutex);
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

        struct timespec now;

        #if __linux__
            status = clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
        #else
            status = clock_gettime(CLOCK_MONOTONIC, &now);
        #endif

        /* we are beyond fucked */
        if(status == -1)
        {   
            watchdog->running = 0;
            break;
        }

        double timeLeft = MAX_TIMEOUT - timespecdiff(&watchdog->last_alive, &now);

        #if DEBUG
            fprintf(stderr, "\033[2K\r[src/watchdog.c:405] by doomTimer(): %.0f ", timeLeft);
            fflush(stderr);
        #endif

        if(watchdog->restart_timer)
        {
            watchdog->restart_timer = 0;
            /* this might drift and be a problem for long sessions... Maybe? */
            watchdog->last_alive.tv_sec += MAX_TIMEOUT - (time_t)ceil(timeLeft);
        }
        else
        {
            const float CHECK_THRESHOLD = .75;

            if(MAX_TIMEOUT * CHECK_THRESHOLD > timeLeft)
            {   
                watchdog->callback();
                Debug0("ru");
            }
        }

        /* have we run out of time? */
        if (timeLeft < 0)
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

    (void)ASSERT(false);

    return EXIT_FAILURE;
}
