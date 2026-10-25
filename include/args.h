#ifndef __WM__ARGCVS__H__
#define __WM__ARGCVS__H__

#include "Argcv/argcv.h"

enum
WMArg
{
    /* THESE MUST HAVE A SHORT AND LONG VERSION
     * OR YOU WILL GET UNDEFINED BEHAVIOR
     */
    WMArgHelp,
    WMArgVersion,

    /* DO NOT PLACE NEW ARGUMENTS HERE */

    /* THESE MUST HAVE ONLY LONG VERSION
     * OR YOU WILL GET UNDEFINED BEHAVIOR
     */
    WMArgDieCat,
    WMArgLicense,
    WMArgSupport,
    WMArgDebug,
    WMArgSkipStartupFile,
    WMArgSkipWMSnCheck,

    /* these args just set their = thing so like --arg=10, so help displays them last */
    WMArgThreads,
    WMArgXDisplay,

    WMArgCount
};

/* Handle command-line arguments */
void WMHandleArgs(int argc, char **argv);
/* Check if an argument was provided and return its ArgOpt pointer if found */
const ArgOpt *WMCheckArg(enum WMArg arg);
/* used to modify args directly DONOT USE UNLESS YOU REALLY KNOW WHAT YOU'RE DOING */
ArgOpt *WMGetArg(enum WMArg arg);

#endif
