#ifndef __SAFE__BOOL__H
#define __SAFE__BOOL__H

/* 
 * if not defined then must be c90 or c89 
 */

/* undef defs */
#ifdef bool
#undef bool
#endif
#ifdef true
#undef true
#endif
#ifdef True
#undef True
#endif
#ifdef false
#undef false
#endif
#ifdef False
#undef False
#endif

typedef enum { false, true } booll;
typedef enum { False, True } boolc;
#if !defined(__STDC_VERSION__)
typedef enum { false, true } bool;
#else
typedef _Bool bool;
#endif

#endif
