/* MIT License
 *
 * Copyright (c) 2024- Joseph
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __SAFE__BOOL__H__
#define __SAFE__BOOL__H__
/* prevent redefinitions of header file */
#ifndef _STDBOOL_H
    #define _STDBOOL_H
#endif
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

#if !defined(__cplusplus)
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
        /* true and false are keywords */
        #define True true
        #define False false
    #else
        typedef enum { False, True } boolc;
        #if !defined(__STDC_VERSION__)
            /* 
             * if not defined then must be c90 or c89 
             */
            typedef enum { false, true } bool;
        #else
            typedef enum { false, true } booll;
            typedef _Bool bool;
        #endif
    #endif
#else
    #define True true
    #define False false
    typedef bool _Bool;
#endif

#ifndef __bool_true_false_are_defined
    #define __bool_true_false_are_defined (1)
#endif  

#endif
