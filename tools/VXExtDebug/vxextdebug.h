#ifndef __VXEXTDEBUG__H__
#define __VXEXTDEBUG__H__

#include <assert.h>

#ifdef __GNUC__
    #define INTERNAL __attribute__((visibility("hidden")))
#else
    #define INTERNAL
#endif

enum
VXMExtDebugType
{
    VXMExtDebugINFO,
    VXMExtDebugDEBUG,
    VXMExtDebugWARN,
    VXMExtDebugERROR,
    VXMExtDebugCRITICAL,
};

extern void INTERNAL vxextdebug(enum VXMExtDebugType type, const char *file, const int line, const char *function, const char *fmt, ...);


#ifndef DebugWarn
    #define DebugWarn(...) vxextdebug(VXMExtDebugWARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef DebugCrit
    #define DebugCrit(...) vxextdebug(VXMExtDebugCRITICAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef DebugError
    #define DebugError(...) vxextdebug(VXMExtDebugERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef NDEBUG

    #ifndef Debug
        #define Debug(...) vxextdebug(VXMExtDebugDEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
    #endif

    #ifndef DebugLog
        #define DebugLog(...) vxextdebug(VXMExtDebugINFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
    #endif

    #ifndef Debug0
        #define Debug0(X) Debug("%s", X)
    #endif

    #ifndef ASSERT
        #define ASSERT(expr) (assert(likely(expr)), likely(expr))
    #endif

#else
    #ifndef Debug
        #define Debug(...) ((void)0)
    #endif

    #ifndef DebugLog
        #define DebugLog(...) ((void)0)
    #endif

    #ifndef Debug0
        #define Debug0(X)       ((void)0)
    #endif

    #ifndef ASSERT
        #define ASSERT(expr)       (likely(expr))
    #endif

#endif


#endif
