#ifndef __FILE__UTIL__H
#define __FILE__UTIL__H

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <errno.h>


/* 
 *
 * NOTE: Recomended buff length atleast 2550 bytes
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
static int
FFGetSysConfigPath(
        char *buff,
        unsigned int buff_len,
        unsigned int *len_return
        )
{
    if(!buff || !buff_len)
    {   return EXIT_FAILURE;
    }
    char *home = NULL;
#ifdef __linux__
    home = getenv("XDG_CONFIG_HOME");
    if(!home)
    {   home = getenv("HOME");
    }
    if(home)
    {   
        const int SNPRINTF_FAILURE = -1;
        const char *const LINUX_CONFIG = "/.config/";
        int len = snprintf(buff, buff_len, "%s%s", home, LINUX_CONFIG);
        if(len == SNPRINTF_FAILURE)
        {   return EXIT_FAILURE;
        }
        if(len_return)
        {   *len_return = len;
        }
        return EXIT_SUCCESS;
    }
#endif
    return EXIT_FAILURE;
}

static inline const unsigned int
FFSysGetConfigPathLengthMAX(
        void
        )
{
    
    const unsigned int MAX_FILENAME = 255;
#ifdef __linux__
    /* Assuming /home/user/.config/mydir/dirname/filename
     * x * 2 to allow for upto x2 layers of config directories for wacky stuff
     */
    const unsigned int PROBABLE_DEPTH = 6;
    const unsigned int MAX_LAYERS = PROBABLE_DEPTH * 2;
    const unsigned int ret = MAX_FILENAME * MAX_LAYERS * sizeof(char);
#endif
    return ret;
}
/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
static int
FFDirExists(
        const char *const DIR_NAME
        )
{
    if(!DIR_NAME)
    {   return EXIT_FAILURE;
    }
    const int NOT_FOUND = -1;
    int statstatus = 0; 
    struct stat st = {0};

    statstatus = stat(DIR_NAME, &st);

    const int EXISTS = statstatus != NOT_FOUND;

    return EXISTS;
}


/* Creates a directory based on path.
 *
 * ex: 
 *      FFCreateDir("home/user/.hidden_dir");
 * 
 * NOTE: Sets errno on failure.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
static int
FFCreateDir(
        const char *const DIR_NAME
        )
{
    if(!DIR_NAME)
    {   return EXIT_FAILURE;
    }
    if(!FFDirExists(DIR_NAME))
    {
        int mkdirstatus = mkdir(DIR_NAME, 0777);
        if(mkdirstatus && errno != EEXIST)
        {   return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}














#endif
