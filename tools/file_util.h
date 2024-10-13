#ifndef __FILE__UTIL__H
#define __FILE__UTIL__H




enum
{   
    FFSysGetConfigPathLengthMAX = 
    /* MAX_FILENAME */
        255 *
#ifdef __linux__
        /* Assuming /home/user/.config/mydir/dirname/filename
         * x * 2 to allow for upto x2 layers of config directories for wacky stuff
         */
        6
#else
        10
#endif
};

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
        );

/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
FFDirExists(
        const char *const DIR_NAME
        );


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
        );

/* Creates a path based on FULL_PATH.
 *
 * ex: 
 *      FFCreatePath("home/user/.hidden_dir");
 * 
 * NOTE: Sets errno on failure.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
FFCreatePath(
        const char *const FULL_PATH
        );

/*
 * RETURN: 1 on Path Exists.
 * RETURN: 0 on Path Doesnt Exists.
 */
int
FFPathExists(
        const char *const FULL_PATH
        );

/*
 * RETURN: 1 on File Exists.
 * RETURN: 0 on File Doesnt Exists.
 */
int
FFFileExists(
        const char *const FILE_NAME
        );

/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
FFCreateFile(
        const char *const FILE_NAME
        );

/*
 * RETURN: 1 on File is Empty.
 * RETURN: 0 on File is not Empty.
 */
int
FFIsFileEmpty(
        const char *const FILE_NAME
        );


#endif
