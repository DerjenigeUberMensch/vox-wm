#ifndef __WM__CONFIG__H__
#define __WM__CONFIG__H__

#include <stdint.h>

/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WMConfigGetFilePath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        );
/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WMConfigGetSessionPath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        );
/*
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int
WMConfigGetSettingsPath(
        char *buff,
        uint32_t buff_length,
        uint32_t *len_return
        );





#endif
