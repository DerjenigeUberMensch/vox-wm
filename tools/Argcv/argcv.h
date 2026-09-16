#ifndef __ARGCV__H__
#define __ARGCV__H__

#include <string.h>
#include <stdbool.h>

typedef struct ArgOpt ArgOpt;

struct
ArgOpt
{
    char *option;
    size_t option_length;
    const char *argument_return;
    bool requires_argument;
    bool found;
};


#define ARGCV_STATIC_INITIALIZER(OPTION_NAME, OPTION_LENGTH, BOOL_REQUIRES_ARGUMENT) { .option = OPTION_NAME, .option_length = OPTION_LENGTH, .requires_argument = BOOL_REQUIRES_ARGUMENT, .argument_return = NULL, .found = false }

/* Initialize an option.
 *
 * NOTE: An intialized option does not have to be destroyed.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
int ArgcvInitialize(ArgOpt *opt_return, char *option, size_t option_length, bool requires_argument);

/* Parse command line arguments.
 *
 * NOTE: This function may overwrite the fields argument_return, and foun. When parsing arguments.
 * NOTE: This argument_return may be NULL if there was no valid argument proceeding the option.
 * NOTE: Usage errors means you passed invalid arguments or you passed invalid options.
 *
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_USAGE on Usage Error.
 * RETURN: EXIT_FAILURE on Failure.
 */
int ArgcvParseArgs(int argc, char **argv, ArgOpt *opts, size_t opt_length, int *end_of_options_return);


#endif
