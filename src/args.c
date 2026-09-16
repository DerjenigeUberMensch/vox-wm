#include "args.h"
#include "Argcv/argcv.h"
#include "khash.h"
#include "main.h"
#include "util.h"
#include <stdlib.h>

typedef struct WMOptions WMOptions;

static ArgOpt ARGS_SHORT[] = 
{
    [WMArgHelp] = ARGCV_STATIC_INITIALIZER("h", 1, false),
    [WMArgVersion] = ARGCV_STATIC_INITIALIZER("v", 1, false),
};

static ArgOpt ARGS_LONG[] = 
{
    [WMArgHelp] = ARGCV_STATIC_INITIALIZER("help", sizeof("help") - 1, false),
    [WMArgVersion] = ARGCV_STATIC_INITIALIZER("version", sizeof("version") - 1, false),
    [WMArgDieCat] = ARGCV_STATIC_INITIALIZER("die-cat", sizeof("die-cat") - 1, false),
    [WMArgLicense] = ARGCV_STATIC_INITIALIZER("license", sizeof("license") - 1, false),
    [WMArgSupport] = ARGCV_STATIC_INITIALIZER("support", sizeof("support") - 1, false),
    [WMArgThreads] = ARGCV_STATIC_INITIALIZER("threads N", sizeof("threads") - 1, true),
    [WMArgSkipStartupFile] = ARGCV_STATIC_INITIALIZER("skip-startup-file", sizeof("skip-startup-file") - 1, false),
};

static char *ARG_DESCRIPTIONS[] = 
{
    [WMArgHelp] = "Help Information",
    [WMArgVersion] = "Version Information",
    [WMArgDieCat] = "Calls 'CATDIE immediately and exits",
    [WMArgLicense] = "Get License Information",
    [WMArgSupport] = "Support Information",
    [WMArgThreads] = "Request a specific number of threads to use for the thread pool",
    [WMArgSkipStartupFile] = "Skip the startup file",
};

void 
WMHandleHelp(const ArgOpt *opt) 
{
    const char *const usage_format = "Usage: " MARK " [OPTION] ...";

    puts(usage_format);
 
    char *format_single = "  -%-20s           %s.\n";
    char *format_double = "  --%-20s          %s.\n";

    size_t i;

    for(i = 0; i < LENGTH(ARGS_SHORT); ++i)
    {   printf(format_single, ARGS_SHORT[i].option, ARG_DESCRIPTIONS[i]);
    }

    puts("");

    for(i = 0; i < LENGTH(ARGS_LONG); ++i)
    {   printf(format_double, ARGS_LONG[i].option, ARG_DESCRIPTIONS[i]);
    }

    exit(EXIT_SUCCESS);
}

void 
WMHandleVersion(const ArgOpt *opt) 
{
    char *compiler = "UNKNOWN";
    char *timestamp = "TimeStamp Not Available";
    short majorversion = -1;
    short minorversion = -1;
    short patchversion = -1;
    int byteorder = GET_BYTE_ORDER();
    int pointersize = sizeof(void *);

#if defined(__GNUC__)
    (void)compiler;
    (void)timestamp;

    compiler = "GCC";
    timestamp = __TIMESTAMP__;
    majorversion = __GNUC__;
    minorversion = __GNUC_MINOR__;
    patchversion = __GNUC_PATCHLEVEL__;
    pointersize = __SIZEOF_POINTER__;
#elif defined(__clang__)
    (void)compiler;
    (void)timestamp;

    compiler = "clang";
    timestamp = __TIMESTAMP__;
    majorversion = __clang_major__;
    minorversion = __clang_minor__;
    patchversion = __clang_patchlevel__;
    pointersize = __SIZEOF_POINTER__;
#elif defined(_MSC_VER)
    (void)compiler;

    compiler = "MSVC";
    majorversion = _MSC_VER;
    minorversion = 0;
    patchversion = 0;
#elif defined(__INTEL__COMPILER)
    (void)compiler;

    compiler = "INTEL";
    majorversion = __INTEL_COMPILER;
    minorversion = 0;
    patchversion = 
    #ifdef __INTEL_COMPILER_UPDATE 
        __INTEL_COMPILER_UPDATE 
    #else 
        0 
    #endif
    ;
#endif

    const int MMM = 3;
    const int SPACE = 1;
    const int DD = 2;

    /* format is MMM <space> DD <space> YYYY
     * which looks like this:        Jan  1 20XX
     * Or like this if double digit: Jan 10 20XX
     * Furthermore we must use [] instead of char * cause its some addr idk, segfault otherwise
     */
    char date[] = __DATE__ "";

    /* This fixes the Jan  1 20XX
     * where theres just a hanging space there 
     * Why are we doing all this work, well basically so printf is the only point of failure here, cause why not.
     * TODO FIXME.
     */
    if(date[MMM + SPACE + DD - 2] == ' ')
    {   date[MMM + SPACE + DD - 2] = '0';
    }

    printf("Compiler Information.\n"
            "  Compiled:        %s %s\n"
            "  Timestamp:       %s\n"
            "  Compiler:        [%s v%d.%d.%d]\n" 
            "  STDC:            [%d] [%d] [%lu]\n"
            "  BYTE_ORDER:      [%d]\n"
            "  POINTER_SIZE:    [%d]\n"
            "Version Information.\n"
            "  VERSION:         [v%d.%d.%d+%d]\n"
            "  COMMIT HASH:     [%s]\n"
            "  MARK:            [%s]\n"
            ,
            /* TODO __DATE__ has an extra space for some reason? */ 
            date, __TIME__,
            timestamp,
            compiler, majorversion, minorversion, patchversion,
            __STDC__, __STDC_HOSTED__, __STDC_VERSION__,
            byteorder,
            pointersize,
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_REVISION,
            VERSION_HASH,
            MARK
          );

    exit(EXIT_SUCCESS);
}

void 
WMHandleDieCat(const ArgOpt *opt) 
{
    DIECAT("%s", "MANUALLY TRIGGERED.");
}

void 
WMHandleLicense(const ArgOpt *opt) 
{
    /* TODO REMOVE LINKS AND JUST PUT THE LICENSE HERE */
    /* TODO THIS LOOKS UGLY */
    puts(
            "--------------------------------LICENSE START--------------------------------------" "\n"
            MARK ":   (MPL-2)  https://www.mozilla.org/en-US/MPL/2.0/" "\n"
            "khash     (MIT)    https://github.com/attractivechaos/klib/blob/master/LICENSE.txt" "\n"
            "thpool    (MIT)    https://github.com/Pithikos/C-Thread-Pool/blob/master/LICENSE" "\n"
            "FNotify   (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "GArray    (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "SCParser  (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "TPromises (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "XCB-TRL   (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "file_util (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "util      (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "safebool  (MIT)    Sub Licensed By DerjenigeUberMensch" "\n"
            "VXExtDebug(MIT)    Sub Licensed by DerjenigeUberMensch" "\n"
            "---------------------------------LICENSE END----------------------------------------"
            );
    exit(EXIT_SUCCESS);
}

void 
WMHandleSupport(const ArgOpt *opt) 
{
    printf(
        "                      [" MARK "]"                                              "\n"
        "|" "----------------------------------------------------" "|"  "\n"
        "| " "MAX NON-SHARED MEM:        [10MiB]                 " "|"   "\n"
        "| " "MAX SHARED MEM:            [1GiB]                  " "|"  "\n"
        "| " "MAX IDLE CPU:              [400 Mhz]               " "|"  "\n"
        "| " "CPU MIN:                   [Intel Core 2 Duo]      " "|"  "\n"
        "| " "CPU RECOMMENDED:           [Intel Core i7-4790]    " "|"  "\n"
        "| " "CPU ARCH:                  [x86_64 Only]           " "|"  "\n"
        "| " "GPU MIN:                   [Any Post 1997]         " "|"  "\n"
        "| " "GPU RECOMMENDED:           [HD Graphics 3000]      " "|"  "\n"
        "|" "----------------------------------------------------" "|"  "\n"
    );
    exit(EXIT_SUCCESS);
}

static void (*WMArgHandler[WMArgCount])(const ArgOpt *)= 
{
    [WMArgHelp] = WMHandleHelp,
    [WMArgVersion] = WMHandleVersion,
    [WMArgDieCat] = WMHandleDieCat,
    [WMArgLicense] = WMHandleLicense,
    [WMArgSupport] = WMHandleSupport,
    [WMArgThreads] = NULL,
    [WMArgSkipStartupFile] = NULL,
};

static void ARGS_CHECK(void)
{
    size_t i;
    const ArgOpt *opt;

    for(i = 0; i < WMArgCount; ++i)
    {
        opt = WMCheckArg((enum WMArg)i);

        if(opt && opt->found)
        {   
            if(WMArgHandler[i])
            {   WMArgHandler[i](opt);
            }
        }
    }
}


void 
WMHandleArgs(int argc, char **argv)
{
    int status;

    status = ArgcvParseArgs(argc, argv, ARGS_SHORT, LENGTH(ARGS_SHORT), NULL);

    if(status != EXIT_SUCCESS)
    {   exit(status);
    }

    status = ArgcvParseArgs(argc, argv, ARGS_LONG, LENGTH(ARGS_LONG), NULL);

    if(status != EXIT_SUCCESS)
    {   exit(status);
    }

    ARGS_CHECK();
}

const ArgOpt *
WMCheckArg(enum WMArg arg)
{
    size_t i;

    ArgOpt *cpy1 = NULL;
    ArgOpt *cpy2 = NULL;

    for(i = 0; i < LENGTH(ARGS_SHORT); ++i)
    {
        if(arg == (enum WMArg)i)
        {   
            cpy1 = &ARGS_SHORT[i];
            break;
        }
    }

    for(i = 0; i < LENGTH(ARGS_LONG); ++i)
    {
        if(arg == (enum WMArg)i)
        {
            cpy2 = &ARGS_LONG[i];
            break;
        }
    }

    if(cpy1 && cpy1->found)
    {   return cpy1;
    }

    if(cpy2 && cpy2->found)
    {   return cpy2;
    }

    return cpy1 ? cpy1 : cpy2;
}