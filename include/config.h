#ifndef __WM__CONFIG__H__
#define __WM__CONFIG__H__

#include <stdint.h>

enum 
WMFolder
{
    WMFolderConfig,
    WMFolderData,
    WMFolderDataBase,
    WMFolderLAST
};

enum
WMFile
{
    WMFileConfig,
    WMFileSession,
    WMFileStartup,
    WMFileLua,
    WMFileManifest,
    WMFileLAST
};

/* Intializes Config path(s).
 * 
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WMConfigInit(
        void
        );
/* Destroys Config path(s) and frees memory.
 * 
 * This function does no return a value.
 */
void
WMConfigDestroy(
        void
        );

/* Returns the path to the specified file.
 * 
 * NOTE: This function may fail due to allocation issues and at no fault of the caller.
 *
 * RETURN: const char * on Success.
 * RETURN: NULL on Failure.
 */
const char *
WMConfigGetPath(
        enum WMFile file
        );

/* Returns the path to the specified folder.
 * 
 * NOTE: This function may fail due to allocation issues and at no fault of the caller.
 *
 * RETURN: const char * on Success.
 * RETURN: NULL on Failure.
 */
const char *
WMConfigGetFolder(
        enum WMFolder folder
        );

#endif
