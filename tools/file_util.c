#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <errno.h>

#include "file_util.h"


/* 
 *
 * NOTE: Recomended buff length atleast 2550 bytes
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
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

/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
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
int
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

int
FFCreatePath(
        const char *const FULL_PATH
        )
{
    return FFCreateDir(FULL_PATH);
}

int
FFPathExists(
        const char *const FULL_PATH
        )
{
    return FFDirExists(FULL_PATH);
}

int
FFFileExists(
        const char *const FILE_NAME
        )
{   return FFPathExists(FILE_NAME);
}

int
FFCreateFile(
        const char *const FILE_NAME
        )
{
    int ret = EXIT_SUCCESS;

    if(!FFPathExists(FILE_NAME))
    {
        FILE *f = fopen(FILE_NAME, "ab+");
        if(!f)
        {   ret = EXIT_FAILURE;
        }
        fclose(f);
    }
    return ret;
}

int
FFIsFileEmpty(
        const char *const FILE_NAME
        )
{
    int ret = 0;

    if(FFPathExists(FILE_NAME))
    {
        FILE *fr = fopen(FILE_NAME, "r");
        if(fr)
        {
            int c = fgetc(fr);
            if(c == EOF)
            {   ret = 1;
            }
        }
        fclose(fr);
    }

    return ret;
}
