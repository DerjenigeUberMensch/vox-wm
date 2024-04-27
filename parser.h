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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SIMPLE_CONFIG_H 
#define SIMPLE_CONFIG_H


#include <stdint.h>
#include <stdlib.h>



typedef struct CFG CFG;
typedef struct CFGItem CFGItem;


struct CFG
{
    char *file;
    CFGItem *items;
    CFGItem *last;
};

struct CFGItem
{
    uint8_t _type;
    size_t size;
    uint32_t _veclen;
    void *data;
    char *name;

    CFGItem *next;
    CFGItem *prev;
};

enum ParseCode
{
    ParseSuccess,
    ParseError,
    ParsePartialError,
    ParseOverflow,
    ParseEOF,
};

enum CFGType
{
    NOTYPE,
    CHAR,
    UCHAR,
    SHORT,
    USHORT,
    INT,
    UINT,
    LONG,
    ULONG,
    FLOAT,
    DOUBLE,
    LASTTYPE,
};


CFG *
CFGCreate(
        char *FILE_NAME
        );

void
CFGDestroy(
        CFG *cfg
        );

CFGItem *
CFGCreateItem(
        void
        );

int
CFGCreateVar(
        CFG *cfg, 
        char *VarName, 
        int CFGType
        );

void *
CFGGetVarValue(
        CFG *cfg, 
        char *VarName
        );

/* Saves data specified by the variable name If it exists. 
 * One must pass in the address of the data wanting to be used, this includes strings interpreted as char *.
 *
 * EX: int x = 10; 
 *     CFGSaveVar(MyCfg, "MyVar", &x);
 * EX: char *str = "my cool string";
 *     char str2[] = "my cool string";
 *     CFGSaveVar(MyCfg, "MyVarString", str);
 *     CFGSaveVar(MyCfg, "MyVarStringArray", str2);
 * 
 *
 * RETURN: 0 On Success.
 * RETURN: 1 On Failure.
 */
int
CFGSaveVar(
        CFG *cfg, 
        char *VarName, 
        void *data
        );

int
CFGWrite(
        CFG *cfg
        );

int
CFGLoad(
        CFG *OldCfg
        );











#endif







