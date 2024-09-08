#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "safebool.h"
#include "argcv.h"

extern WM _wm;

bool
ArgcvIsSingleCommand(
        char *str,
        unsigned long len
        )
{
    const unsigned char MIN_LENGTH = 2;     /* -X */
    const unsigned char SINGLE_OPTION_INDEX_DASH = 0;
    const unsigned char SINGLE_OPTION_INDEX_LETTER = 1;


    const bool LONG_ENOUGH = len == MIN_LENGTH;
    const bool VALID_DASH = str[SINGLE_OPTION_INDEX_DASH] == '-';
    const bool VALID_CHAR = isalpha(str[SINGLE_OPTION_INDEX_LETTER]);

    return LONG_ENOUGH && VALID_DASH && VALID_CHAR;
}

bool
ArgcvIsDoubleCommand(
        char *str,
        unsigned long len
        )
{
    const unsigned char MIN_LENGTH = 3;
    const unsigned char DOUBLE_OPTION_INDEX_DASH_FIRST = 0;
    const unsigned char DOUBLE_OPTION_INDEX_DASH_SECOND = 1;
    const unsigned char DOUBLE_OPTION_INDEX_STRING_START = 2;

    const bool LONG_ENOUGH = len >= MIN_LENGTH;
    const bool VALID_DASHES = str[DOUBLE_OPTION_INDEX_DASH_FIRST] == '-' && str[DOUBLE_OPTION_INDEX_DASH_SECOND] == '-';
    const bool VALID_CHAR = isalpha(str[DOUBLE_OPTION_INDEX_STRING_START]);

    return LONG_ENOUGH && VALID_DASHES && VALID_CHAR;
}

void
ArgcvDisplayHelp(
        void
        )
{
    puts( "Usage: " MARK " [options]\n"
            "  -h           Help Information.\n"
            "  -v           Compiler Information."
          );
}

void
ArgcvDisplayCompilerInfo(
        void
        )
{
    char *compiler;
    short majorversion = -1;
    short minorversion = -1;
    short patchversion = -1;
    compiler = "UNKNOWN";
#if defined(__GNUC__)
    compiler = "GCC";
    majorversion = __GNUC__;
    minorversion = __GNUC_MINOR__;
    patchversion = __GNUC_PATCHLEVEL__;
#elif defined(__clang__)
    compiler = "clang";
    majorversion = __clang_major__;
    minorversion = __clang_minor__;
    patchversion = __clang_patchlevel__;
#elif defined(_MSC_VER)
    compiler = "MSVC";
    majorversion = _MSC_VER;
    minorversion = 0;
    patchversion = 0;
#elif defined(__INTEL__COMPILER)
    compiler = "INTEL";
    majorversion = __INTEL_COMPILER;
    minorversion = 0;
    patchversion = #ifdef __INTEL_COMPILER_UPDATE __INTEL_COMPILER_UPDATE #else 0 #endif;
#endif
    printf( "Compiler Information.\n"
            "  Compiled:        %s %s\n"
            "  Compiler:        [%s v%d.%d.%d]\n" 
            "  STDC:            [%d] [%lu]\n"
            "  BYTE_ORDER:      [%d]\n"
            "  POINTER_SIZE:    [%d]\n"
            "Version Information.\n"
            "  VERSION:         [%s]\n"
            "  MARK:            [%s]\n"
            ,
            /* TODO __DATE__ has an extra space for some reason? */ 
            __DATE__, __TIME__,
            compiler, majorversion, minorversion, patchversion,
            __STDC_HOSTED__, __STDC_VERSION__,
            __BYTE_ORDER__,
            __SIZEOF_POINTER__,
            VERSION,
            MARK
          );
}

void
ArgcvDisplayBadArgs(
        char *arg_str
        )
{
    printf("UNKNOWN COMMAND: \'%s\'\n", arg_str);
}

void
ArgcvSingleCommandHandler(
        char *str
        )
{
    const unsigned char SINGLE_OPTION_INDEX_LETTER = 1;

    switch(str[SINGLE_OPTION_INDEX_LETTER])
    {
        case 'h':
            ArgcvDisplayHelp();
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            ArgcvDisplayCompilerInfo();
            exit(EXIT_SUCCESS);
            break;
        default:
            ArgcvDisplayBadArgs(str);
            break;
    }
}

void
ArgcvDoubleCommandHandler(
        char *str
        )
{
    const unsigned char DOUBLE_OPTION_INDEX_STR = 2;
    char *option = str + DOUBLE_OPTION_INDEX_STR;

    /* PLACEHOLDER */
    if(!strcmp(option, "NULL"))
    {   (void)0;
    }
    else
    {   ArgcvDisplayBadArgs(str);
    }
}


void
ArgcvHandler(
        int argc, 
        char **argv
        )
{
    unsigned int i;
    unsigned long len;

    for(i = 0; i < argc; ++i)
    {
        if(!argv[i])
        {   continue;
        }
        len = strlen(argv[i]);
        if(ArgcvIsSingleCommand(argv[i], len))
        {   ArgcvSingleCommandHandler(argv[i]);
        }
        else if(ArgcvIsDoubleCommand(argv[i], len))
        {   ArgcvDoubleCommandHandler(argv[i]);
        }
        else
        {   
            const char exec1 = '.';
            const char exec2 = '/';
            const char execcount = 3; /* not 2 because we need \0 */ /* +1 for the possible 1 letter name and +1 again for \0   */
            if(argv[0] != NULL && strnlen(argv[0], execcount + 2) >= execcount && argv[0][0] == exec1 && argv[0][1] == exec2)
            {   
                /* We can call die because it is very likely this was run manually */
                if(i > 0)
                {
                    ArgcvDisplayHelp();
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {   /* We dont die because likely this command was run using some form of exec */
                ArgcvDisplayBadArgs(argv[i]);
            }
        }
    }
}


