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
    WMArgSkipStartupFile,

    /* these args just set their = thing so like --arg=10, so help displays them last */
    WMArgThreads,
    WMArgXDisplay,

    WMArgCount
};

void WMHandleArgs(int argc, char **argv);
const ArgOpt *WMCheckArg(enum WMArg arg);

#endif