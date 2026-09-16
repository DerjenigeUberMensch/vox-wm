#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "argcv.h"

static bool IS_DOUBLE_HYPHEN_END(const char *str, size_t len)  {   return len == 2 && str[0] == '-' && str[1] == '-';  }
static bool IS_DOUBLE_OPTION(const char *str, size_t len)      {   return len >  2 && str[0] == '-' && str[1] == '-';   }
static bool IS_SINGLE_OPTION(const char *str, size_t len)      {   return len >= 2 && str[0] == '-' && isalpha((unsigned char)str[1]); }

int 
ArgcvInitialize(ArgOpt *opt_return, char *option, size_t option_length, bool requires_argument)
{
    if(!opt_return)
    {   return EXIT_FAILURE;
    }
    if(!option)
    {   return EXIT_FAILURE;
    }
    if(!option_length)
    {   return EXIT_FAILURE;
    }

    opt_return->option = option;
    opt_return->option_length = option_length;
    opt_return->requires_argument = requires_argument;
    opt_return->argument_return = NULL;
    opt_return->found = false;

    return EXIT_SUCCESS;
}

int
ArgcvParseArgs(int argc, char **argv, ArgOpt *opts, size_t opt_length, int *end_of_options_return)
{
    enum { EXIT_USAGE = 2 };

    if(argc <= 0)
    {   return EXIT_USAGE; 
    }

    if(!argv)
    {   return EXIT_USAGE;
    }

    if(argc == 1)
    {   return EXIT_SUCCESS;
    }

    if(!argv[0])
    {   return EXIT_USAGE;
    }

    if(opts)
    {
        size_t i;

        for(i = 0; i < opt_length; ++i)
        {
            if(!opts[i].option || opts[i].option_length == 0)
            {   return EXIT_USAGE;
            }

            opts[i].found = false;
            opts[i].argument_return = NULL;
        }
    }

    if((!opts || opt_length == 0) && !end_of_options_return)
    {   return EXIT_USAGE;
    }

    int i;
    bool is_arg = false;
    int opti = 0;

    for(i = 1; i < argc; ++i)
    {
        if(!argv[i])
        {   return EXIT_USAGE;
        }

        const char *str = argv[i];
        size_t len = strlen(str);

        if(IS_DOUBLE_HYPHEN_END(str, len))
        {   
            if(end_of_options_return)
            {   *end_of_options_return = i;
            }
            goto END;
        }

        if(!opts || opt_length == 0)
        {   goto SKIP;
        }

        if(is_arg)
        {
            opts[opti].argument_return = argv[i];
            is_arg = false;
            goto SKIP;
        }

        if(IS_DOUBLE_OPTION(str, len))
        {
            const char *opt_real = str + 2;
            size_t opt_str_length = len - 2;

            size_t j;

            for(j = 0; j < opt_length; ++j)
            {
                size_t opt_user_length = opts[j].option_length;

                if(opt_user_length == opt_str_length && strcmp(opts[j].option, opt_real) == 0)
                {
                    if(opts[j].requires_argument)
                    {
                        is_arg = true;
                        opti = j;
                        opts[j].argument_return = NULL;
                    }

                    opts[j].found = true;

                    goto SKIP;
                }
                else if(opt_user_length + 1 <= opt_str_length)
                {
                    if(opts[j].requires_argument &&
                        opt_real[opt_user_length] == '=' && strncmp(opts[j].option, opt_real, opt_user_length) == 0)
                    {   
                        opts[j].argument_return = opt_real + (opts[j].option_length + 1);
                        opts[j].found = true;
                        goto SKIP;
                    }
                }
            }
        }

        if(IS_SINGLE_OPTION(str, len))
        {
            const char *opt_real = str + 1;
            size_t opt_str_length = len - 1;

            size_t j;
            size_t k;

            for(j = 0; j < opt_str_length; ++j)
            {
                for(k = 0; k < opt_length; ++k)
                {
                    if(opts[k].option[0] == opt_real[j] && opts[k].option_length == 1)
                    {
                        opts[k].found = true;

                        if(opts[k].requires_argument)
                        {
                            if(j + 1 < opt_str_length)
                            {   
                                opts[k].argument_return = opt_real + j + 1;
                            }
                            else
                            {
                                is_arg = true;
                                opti = k;
                                opts[opti].argument_return = NULL;
                            }
                            goto SKIP;
                        }
                    }
                }
            }
        }
SKIP:   ;
    }

END:
    if(is_arg)
    {   return EXIT_USAGE;
    }

    return EXIT_SUCCESS;
}