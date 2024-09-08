#ifndef __ARGCV__H__
#define __ARGCV__H__

#include "safebool.h"

bool
ArgcvIsSingleCommand(
        char *str,
        unsigned long len
        );

bool
ArgcvIsDoubleCommand(
        char *str,
        unsigned long len
        );

void
ArgcvDisplayHelp(
        void
        );

void
ArgcvDisplayCompilerInfo(
        void
        );

void
ArgcvDisplayBadArgs(
        char *arg_str
        );

void
ArgcvSingleCommandHandler(
        char *str
        );

void
ArgcvDoubleCommandHandler(
        char *str
        );

void
ArgcvHandler(
        int argc, 
        char **argv
        );



#endif
