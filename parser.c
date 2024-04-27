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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"



typedef union __Generic __Generic;

union __Generic
{
    void *v;
    void **vv;

    uint8_t uc;
    int8_t c;
    uint16_t us;
    int16_t s;
    int32_t i;
    uint32_t ui;
    int64_t ii;
    uint64_t uii;

    float f;
    double d;
};

static int /* FILE READ,   Buffer FILL data, Buffer Length */
__FILE_GET_NEW_LINE(FILE *fr, char *buff, unsigned int bufflength)
{
    char *nl;
    if(fgets(buff, bufflength, fr))
    {
        nl = strchr(buff, '\n');
        if(!nl)
        {
            if(!feof(fr))
            {   return ParseOverflow;
            }
        } /* remove new line char */
        else
        {   *nl = '\0';
        }
        return ParseSuccess;
    }
    else
    {
        if(ferror(fr))
        {
            return ParseError;
        }
        return ParseEOF;
    }
}


static void
__attach(CFG *cfg, CFGItem *item)
{
    item->next = cfg->items;
    cfg->items = item;
    if(item->next)
    {
        item->next->prev = item;
    }
    else
    {
        cfg->last = item;
    }
    item->prev = NULL;
}

static void
__detach(CFG *cfg, CFGItem *item)
{
    CFGItem **tc;
    for(tc = &cfg->items; *tc && *tc != item; tc = &(*tc)->next);
    *tc = item->next;
    if(!(*tc))
    {
        cfg->last = item->prev;
    }
    else if(item->next)
    {
        item->next->prev = item->prev;
    }
    else if(item->prev)
    {
        item->prev->next = NULL;
    }

    item->prev = NULL;
    item->next = NULL;
}



static char *
__REMOVE_WHITE_SPACE(char *str, int str_len)
{
    char *ret = NULL;
    int reti;
    int i;
    const int len = str_len;
    int size = len + sizeof(char);
    ret = malloc(size);
    if(!ret)
    {   return NULL;
    }
    reti = 0;
    for(i = 0; i < len; ++i)
    {   
        if(str[i] != ' ')
        {   
            ret[reti] = str[i];
            ++reti;
        }
    }
    if(reti < size)
    {   
        size = reti + sizeof(char);
        void *tmp = realloc(ret, size);
        if(!tmp)
        {   
            free(ret);
            return NULL;
        }
        ret = tmp;
    }
    ret[size - 1] = '\0';
    return ret;
}

/* Allocated mememory must be freed by caller.
 * Can return NULL
 */
static char *
__CFG_PARSE_NAME(char *buff)
{
    const char *delimeter = "=";
    char *token = NULL;
    char *ret = NULL;
    const int maxlen = 1024;
    
    token = strtok(buff, delimeter);
    if(token)
    {   ret = __REMOVE_WHITE_SPACE(token, strnlen(token, maxlen));
    }
    return ret;
}

static char *
__CFG_PARSE_VALUE_STR(char *buff)
{
    const char *delimeter = "=";
    char *token = NULL;
    char *ret = NULL;
    const int maxlen = 1024;
    
    token = strtok(buff, delimeter);
    if(token)
    {   ret = __REMOVE_WHITE_SPACE(token, strnlen(token, maxlen));
    }
    return ret;
}


static CFGItem *
__CFG_GET_VAR_FROM_STRING(CFG *cfg, const char *VarName)
{
    CFGItem *item;
    for(item = cfg->items; item && strcmp(item->name, VarName); item = item->next);
    return item;
}

static char *
__CFG_GET_FORMAT_SPECIFIER_FROM_TYPE(int CFGType)
{
     switch(CFGType)
    {
        case INT:
            return "%d";
        case UINT:
            return "%u";
        case LONG:
            return "%ld";
        case ULONG:
            return "%lu";
        case FLOAT:
            return "%f";
        case DOUBLE:
            return "%lf";
        case CHAR:
            return "%c";
        case UCHAR:
            return "%d";
    }
    return NULL;  
}

static size_t
__CFG_GET_TYPE_SIZE(int CFGType)
{
    switch(CFGType)
    {
        case INT:
            return sizeof(int);
        case UINT:
            return sizeof(unsigned int);
        case LONG:
            return sizeof(long int);
        case ULONG:
            return sizeof(unsigned long int);
        case FLOAT:
            return sizeof(float);
        case DOUBLE:
            return sizeof(double);
        case CHAR:
            return sizeof(char);
        case UCHAR:
            return sizeof(unsigned char);
    }
    //fprintf(stderr, "Allocating 0 Bytes? are you sure you have a type set.");
    return sizeof(void *);
}



CFG *
CFGCreate(
        char *FILE_NAME
        )
{
    CFG *ret = malloc(sizeof(CFG));

    if(ret)
    {   
        ret->file = FILE_NAME;
        ret->items = NULL;
    }

    return ret;
}

void
CFGDestroy(
    CFG *cfg
    )
{
    CFGItem *prev;
    CFGItem *current = cfg->items;
    while(current)
    {   
        prev = current; 
        current = current->next;
        free(prev->data);
        free(prev);
    }
    free(cfg);
}


CFGItem *
CFGCreateItem(
        void
        )
{
    CFGItem *item = malloc(sizeof(CFGItem));

    if(item)
    {
        item->data = NULL;
        item->_type = 0;
        item->name = NULL;
        item->next = NULL;
        item->prev = NULL;
        item->_veclen = 0;
    }

    return item;
}

int
CFGCreateVar(
        CFG *cfg, 
        char *VarName, 
        int CFGType)
{
    CFGItem *item = CFGCreateItem();

    if(item)
    {
        item->data = calloc(1, sizeof(__Generic));
        if(!item->data)
        {   
            free(item->data);
            free(item);
            return 1;
        }
        item->name = VarName;
        item->_type = CFGType;
        item->size = __CFG_GET_TYPE_SIZE(CFGType);
        __attach(cfg, item);
    }
    return !item;
}

void *
CFGGetVarValue(
        CFG *cfg, 
        char *VarName
        )
{
    CFGItem *item;
    if((item = __CFG_GET_VAR_FROM_STRING(cfg, VarName)))
    {   return item->data;
    }
    return NULL;
}

/* Saves data specified by the variable name If it exists. 
 * One must pass in the address of the data wanting to be used, this includes strings interpreted as char *.
 * Do note that the data type must be correct as no bounds checks are made when copying memoery.
 *
 * EX: int x = 10; 
 *     CFGSaveVar(MyCfg, "MyVar", &x);
 * EX: char *str = "my cool string";
 *     char str2[] = "my cool string";
 *     CFGSaveVar(MyCfg, "MyVarString", &str);
 *     CFGSaveVar(MyCfg, "MyVarStringArray", &str2);
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
        )
{
    CFGItem *item;
    if((item = __CFG_GET_VAR_FROM_STRING(cfg, VarName)))
    {   
        switch(item->_type)
        {
            case INT:
                ((__Generic *)item->data)->i = *(int32_t *)data;
                break;
            case UINT:
                ((__Generic *)item->data)->ui = *(uint32_t *)data;
                break;
            case LONG:
                ((__Generic *)item->data)->ii = *(int64_t *)data;
                break;
            case ULONG:
                ((__Generic *)item->data)->uii = *(uint64_t *)data;
                break;
            case FLOAT:
                ((__Generic *)item->data)->f = *(float *)data;
                break;
            case DOUBLE:
                ((__Generic *)item->data)->d = *(double *)data;
                break;
            case CHAR:
                ((__Generic *)item->data)->c = *(int8_t *)data;
                break;
            case UCHAR:
                ((__Generic *)item->data)->uc = *(uint8_t *)data;
                break;
        }
    }
    return !item;
}

int
CFGWrite(
        CFG *cfg
        )
{
    CFGItem *item;
    char *format;
    __Generic *data;
    FILE *fw = fopen(cfg->file, "w");

    if(!fw)
    {   return ParseError;
    }
    uint8_t error = ParseSuccess;
    for(item = cfg->last; item; item = item->prev)
    {
        format = __CFG_GET_FORMAT_SPECIFIER_FROM_TYPE(item->_type);
        data = item->data;
        fprintf(fw, "%s=", item->name);
        switch(item->_type)
        {
            case INT:
                fprintf(fw, format, data->i);
                break;
            case UINT:
                fprintf(fw, format, data->ui);
                break;
            case LONG:
                fprintf(fw, format, data->ii);
                break;
            case ULONG:
                fprintf(fw, format, data->uii);
                break;
            case FLOAT:
                fprintf(fw, format, data->f);
                break;
            case DOUBLE:
                fprintf(fw, format, data->d);
                break;
            case CHAR:
                fprintf(fw, format, data->c);
                break;
            case UCHAR:
                fprintf(fw, format, data->uc);
                break;
        }
        fprintf(fw, "\n");
    }
    fclose(fw);
    return error;
}

int
CFGLoad(
        CFG *OldCfg
        )
{
    const int standardbufflimit = 1024 << 2;
    int running = 1;
    char buff[standardbufflimit];
    FILE *fr = fopen(OldCfg->file, "r");

    char *name = NULL;
    char *typename = NULL;
    void *data = NULL;
    if(!fr)
    {   return ParseError;
    }

    uint8_t error = ParseSuccess;
    
    while(running)
    {
        switch(__FILE_GET_NEW_LINE(fr, buff, standardbufflimit))
        {
            case ParseSuccess: 
                break;
            case ParseEOF:
                running = 0;
                /* FALLTHROUGH */
            case ParseOverflow: case ParseError: default:
                continue;
        }
        name = __CFG_PARSE_NAME(buff);
        typename = __CFG_PARSE_VALUE_STR(NULL);
        if(!name || !typename)
        {   
            free(name);
            free(typename);
            continue;
        }
        CFGItem *item;
        char *format = NULL;
        if((item = __CFG_GET_VAR_FROM_STRING(OldCfg, name)))
        {
            if((format = __CFG_GET_FORMAT_SPECIFIER_FROM_TYPE(item->_type)))
            { 
                __Generic ge;
                const uint8_t SSCANF_SUCCESS = 1;
                const uint8_t sscanfstatus = sscanf(typename, format, &ge);
                if(SSCANF_SUCCESS == sscanfstatus)
                {   CFGSaveVar(OldCfg, name, &ge);
                }
            }
        }
        free(name);
        free(typename);
    }
    fclose(fr);
    return error;
}

