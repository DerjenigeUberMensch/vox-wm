#ifndef __SAFE__BOOL__H__
#define __SAFE__BOOL__H__

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


#ifndef Nill
#define Nill ((void *)0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef null
#define null ((void *)0)
#endif


typedef enum { false, true } booll;
typedef enum { False, True } boolc;

/* 
 * if not defined then must be c90 or c89 
 */

#if !defined(__STDC_VERSION__)
typedef enum { false, true } bool;
#else
typedef _Bool bool;
#endif

#endif
