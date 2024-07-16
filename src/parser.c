/* MIT License
 *
 * Copyright (c) 2024 Joseph
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>  /* isdigit() */

#include "parser.h"


#include "khash.h"

/* hashing */
KHASH_MAP_INIT_STR(__STR__TABLE__, uint32_t)

struct 
_SP_PARSER_STRUT
{
    SCItem *items;
    uint32_t item_len;
    uint32_t index;
    khash_t(__STR__TABLE__) *strtable;
};

struct
_SP_PARSER_ITEM
{
    char *name;
    char *typename;
    void *data;
    uint32_t name_len;
    uint32_t type_len;
    uint8_t allocated;
    uint8_t type;
    uint16_t size;
};

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
        /* comment */
        if(str[i] == '#')   
        {   break;
        }
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
__SC_PARSE_NAME(char *buff)
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
__SC_PARSE_VALUE_STR(char *buff)
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
        {   return ParseError;
        }
        return ParseEOF;
    }
}

static const char *const
__SC_GET_FORMAT_FROM_TYPE(const enum SCType t)
{
    switch(t)
    {
        case SCTypeNoType:
            return NULL;
        case SCTypeCHAR:
            return "%c";
        case SCTypeUCHAR:
            return "%d";
        case SCTypeSHORT:
            return "%d";
        case SCTypeUSHORT:
            return "%d";
        case SCTypeINT:
            return "%d";
        case SCTypeUINT:
            return "%u";
        case SCTypeFLOAT:
            return "%f";
        case SCTypeDOUBLE:
            return "%lf";
        case SCTypeLONG:
            return "%ld";
        case SCTypeULONG:
            return "%lu";
        case SCTypeSTRING:
            return "%s";
    }
    return NULL;
}

static const char *const 
__SC_GET_FORMAT_FROM_SIZE(const size_t size)
{
    switch(size)
    {
        case sizeof(int8_t):
            return "%c";
        case sizeof(int16_t):
            return "%d";
        case sizeof(int32_t):
            return "%f";
        case sizeof(int64_t):
            return "%ld";
    }
    return "%c";
}

static const unsigned int
__SC_GET_SIZE_FROM_TYPE(const enum SCType t)
{
    switch(t)
    {
        case SCTypeNoType:
            return 0;
        case SCTypeCHAR:
            return sizeof(int8_t);
        case SCTypeUCHAR:
            return sizeof(uint8_t);
        case SCTypeSHORT:
            return sizeof(int16_t);
        case SCTypeUSHORT:
            return sizeof(uint16_t);
        case SCTypeINT:
            return sizeof(int32_t);
        case SCTypeUINT:
            return sizeof(uint32_t);
        case SCTypeFLOAT:
            return sizeof(float);
        case SCTypeDOUBLE:
            return sizeof(double);
        case SCTypeLONG:
            return sizeof(int64_t);
        case SCTypeULONG:
            return sizeof(uint64_t);
        case SCTypeSTRING:
            return 0;
    }
    return 0;
}

SCItem *
SCParserSearch(
        SCParser *parser,
        const char *const NAME
        )
{
    if(!parser)
    {   return NULL;
    }
    khint_t k = kh_get(__STR__TABLE__, parser->strtable, NAME);
    if(k != kh_end(parser->strtable))
    {   
        uint32_t index = kh_val(parser->strtable, k);
        if(parser->index > index && index >= 0)
        {   return parser->items + index;
        }
    }
    return NULL;
}

SCItem *
SCParserSearchSlow(
        SCParser *parser,
        const char *const NAME
        )
{
    if(!parser || !NAME)
    {   return NULL;
    }
    uint32_t i;
    SCItem *item;
    for(i = 0; i < parser->index; ++i)
    {
        item = parser->items + i;
        if(!strcmp(item->name, NAME))
        {   return item;
        }
    }
    return NULL;
}

int
SCParserLoad(
        SCItem *item,
        void *_return,
        const size_t bytescopy,
        const enum SCType _optional_type
        )
{
    const int FAILURE = 1;
    const int SUCCESS = 0;
    if(!item || !item->typename || !_return)
    {   return FAILURE;
    }

    const char *const format = __SC_GET_FORMAT_FROM_TYPE(_optional_type);
    const int SSCANF_CHECKSUM = 1;
    const int DATA_SIZE = 32;
    uint8_t check;
    uint8_t data[DATA_SIZE];

    /* type handler */
    if(_optional_type == SCTypeNoType)
    {   goto NOTYPE;
    }

    /* else we got a type */
    if(_optional_type == SCTypeSTRING)
    {
        const uint32_t len = item->type_len;
        const size_t size = len * sizeof(char);
        char *str = malloc(size);
        if(str)
        {   
            memcpy(str, item->typename, size);
            memcpy(_return, &str, sizeof(char *));
            return SUCCESS;
        }
        return FAILURE;
    }
    if(!format)
    {   goto NOTYPE;
    }
    /* clear memory */
    memset(data, 0, sizeof(uint8_t) * DATA_SIZE);
    check = sscanf(item->typename, format, &data);
    if(check == SSCANF_CHECKSUM)
    {   
        size_t copysize = bytescopy;
        if(bytescopy > __SC_GET_SIZE_FROM_TYPE(item->type))
        {   copysize = __SC_GET_SIZE_FROM_TYPE(item->type);
        }
        memcpy(_return, data, copysize);
        return SUCCESS;
    }
    return FAILURE;
NOTYPE:
    /* TODO fix this later, dont use it though, prob when I use it again */
    /* check if negative */
    memset(data, 0, sizeof(uint8_t) * DATA_SIZE);
    check = sscanf(item->typename, __SC_GET_FORMAT_FROM_SIZE(item->size), data);
    if(check == SSCANF_CHECKSUM)
    {
        memcpy(_return, data, bytescopy);
        return SUCCESS;
    }
    return FAILURE;
}


int
SCParserWrite(
        SCParser *parser,
        const char *const FILE_NAME
        )
{
    const int SUCCESS = 0;
    const int FAILURE = 1;
    if(!parser)
    {   return FAILURE;
    }

    FILE *fw = fopen(FILE_NAME, "w");

    if(!fw)
    {   return FAILURE;
    }

    SCItem *item;
    uint32_t i;

    for(i = 0; i < parser->index; ++i)
    {
        item = parser->items + i;
        if(item->data && item->name)
        {   
            const char *const format = __SC_GET_FORMAT_FROM_TYPE(item->type);
            fprintf(fw, "%s=", item->name);
            if(format)
            {   
                switch(item->type)
                {
                    case SCTypeCHAR:
                        fprintf(fw, format, *(int8_t *)item->data);
                        break;
                    case SCTypeUCHAR:
                        fprintf(fw, format, *(uint8_t *)item->data);
                        break;
                    case SCTypeSHORT:
                        fprintf(fw, format, *(int16_t *)item->data);
                        break;
                    case SCTypeUSHORT:
                        fprintf(fw, format, *(uint16_t *)item->data);
                        break;
                    case SCTypeINT:
                        fprintf(fw, format, *(int32_t *)item->data);
                        break;
                    case SCTypeUINT:
                        fprintf(fw, format, *(uint32_t *)item->data);
                        break;
                    case SCTypeFLOAT:
                        fprintf(fw, format, *(float *)item->data);
                        break;
                    case SCTypeDOUBLE:
                        fprintf(fw, format, *(double *)item->data);
                        break;
                    case SCTypeLONG:
                        fprintf(fw, format, *(int64_t *)item->data);
                        break;
                    case SCTypeULONG:
                        fprintf(fw, format, *(uint64_t *)item->data);
                        break;
                    case SCTypeSTRING:
                        fprintf(fw, format, (char *)item->data);
                        break;
                }
            }
            else
            {   
                switch(item->size)
                {
                    case sizeof(int8_t):
                        fprintf(fw, __SC_GET_FORMAT_FROM_SIZE(item->size), *(char *)item->data);
                        break;
                    case sizeof(int16_t):
                        fprintf(fw, __SC_GET_FORMAT_FROM_SIZE(item->size), *(int16_t *)item->data);
                        break;
                    case sizeof(int32_t):
                        fprintf(fw, __SC_GET_FORMAT_FROM_SIZE(item->size), *(float *)item->data);
                        break;
                    case sizeof(int64_t):
                        fprintf(fw, __SC_GET_FORMAT_FROM_SIZE(item->size), *(int64_t *)item->data);
                        break;
                }
            }
            fprintf(fw, "\n");
        }
    }
    fclose(fw);
    return SUCCESS;
}


SCParser * 
SCPParserCreate(
        const uint32_t BASE_VAR_COUNT
        )
{
    SCParser *p = malloc(sizeof(SCParser));
    if(p)
    {
        p->strtable = kh_init(__STR__TABLE__);

        if(!p->strtable)
        {   
            free(p);
            return NULL;
        }
        p->items = malloc(BASE_VAR_COUNT * sizeof(SCItem));
        if(!p->items)
        {
            kh_destroy(__STR__TABLE__, p->strtable);
            free(p);
            return NULL;
        }
        p->item_len = BASE_VAR_COUNT;
        p->index = 0;
    }
    return p;
}

void
SCParserDestroy(
        SCParser *parser
        )
{
    uint32_t i;
    SCItem *item;
    for(i = 0; i < parser->index; ++i)
    {
        item = parser->items + i;
        if(item->allocated)
        {   free(item->name);
        }
        if(item->typename)
        {   free(item->typename);
        }
        free(item->data);
    }
    kh_destroy(__STR__TABLE__, parser->strtable);
    free(parser->items);
    free(parser);
}

int
SCParserReadFile(
        SCParser *parser,
        const char *const FILE_NAME
        )
{
    const int FAILURE = 1;
    const int SUCCESS = 0;

    if(!parser)
    {   return FAILURE;
    }


    FILE *fr = fopen(FILE_NAME, "r");

    if(!fr)
    {   return FAILURE;
    }

    const int BUFF_LIMIT = 1024;
    int running = 1;
    char buff[BUFF_LIMIT];
    char *name = NULL;
    char *typename = NULL;

    SCItem *item;
    while(running)
    {
        switch(__FILE_GET_NEW_LINE(fr, buff, BUFF_LIMIT))
        {
            case ParseSuccess: 
                break;
            case ParseEOF:
                running = 0;
                /* FALLTHROUGH */
            case ParseOverflow: 
            case ParseError: 
            default:
                continue;
        }
        name = __SC_PARSE_NAME(buff);
        typename = __SC_PARSE_VALUE_STR(NULL);
        if(!name || !typename)
        {   
            free(name);
            free(typename);
            continue;
        }

        item = SCParserSearch(parser, name);
        if(!item)
        {   item = SCParserSearchSlow(parser, name);
        }
        if(item)
        {   
            if(item->typename)
            {   free(item->typename);
            }
            item->typename = typename;
            item->type_len = strlen(typename);
        }
        else
        {   free(typename);
        }
        free(name);
    }

    fclose(fr);
    return SUCCESS;
}

int
SCParserNewVar(
        SCParser *parser,
        const char *const VAR_NAME,
        const uint32_t VAR_NAME_FULL_LENGTH,
        const uint8_t READONLY_SECTION,
        const size_t size,
        const enum SCType _optional_type
        )
{
    const int FAILURE = 1;
    const int SUCCESS = 0;
    if(!parser || !VAR_NAME)
    {   return FAILURE;
    }
    if(!size && !__SC_GET_FORMAT_FROM_TYPE(_optional_type))
    {   return FAILURE;
    }

    /* increase array size if too small */
    if(parser->index >= parser->item_len)
    {
        const float INCREASE_FACTOR = 1.5f;
        const uint32_t newlen = parser->item_len * INCREASE_FACTOR;
        void *rec = realloc(parser->items, (newlen) * sizeof(SCItem));
        if(rec)
        {
            parser->items = rec;
            parser->item_len = newlen;
        }
        else
        {   return FAILURE;
        }
    }

    SCItem *item = parser->items + parser->index;

    if(_optional_type == SCTypeSTRING)
    {
        item->size = sizeof(char *);
        item->type = SCTypeSTRING;
    }
    else if(__SC_GET_FORMAT_FROM_TYPE(_optional_type))
    {   
        item->size = __SC_GET_SIZE_FROM_TYPE(_optional_type);
        item->type = _optional_type;
    }
    else if(size)
    {
        item->size = size;
        item->type = SCTypeNoType;
    }
    else
    {   return FAILURE;
    }

    if(READONLY_SECTION)
    {
        item->name = VAR_NAME;
        item->allocated = 0;
    }
    else
    {
        item->name = malloc(VAR_NAME_FULL_LENGTH * sizeof(char));
        item->allocated = 1;
        if(!item->name)
        {   return FAILURE;
        }
        memcpy(item->name, VAR_NAME, VAR_NAME_FULL_LENGTH);
    }
    item->name_len = VAR_NAME_FULL_LENGTH;
    item->data = malloc(item->size);
    item->typename = NULL;
    item->type_len = 0;

    /* add to table */
    int err = 3;
    khint_t k = kh_put(__STR__TABLE__, parser->strtable, item->name, &err);
    enum
    {
        __KHASH_BAD_OPERATION = -1,
        __KHASH_ALREADY_PRESENT = 0,
        __KHASH_FIRST_HASH = 1,
        __KHASH_PREVIOUSLY_DELETED = 2,
    };
    /* set value to client */
    switch(err)
    {
        case __KHASH_BAD_OPERATION:
            /* FALLTHROUGH */
        case __KHASH_ALREADY_PRESENT:
            break;

        case __KHASH_FIRST_HASH:
            /* FALLTHROUGH */
        case __KHASH_PREVIOUSLY_DELETED:
            /* FALLTHROUGH */
        default:
            /* bounds check */
            if(kh_end(parser->strtable) > k)
            {   kh_value(parser->strtable, k) = parser->index;
            }
            break;
    }
    ++parser->index;
    return SUCCESS;
}

int
SCParserDelVar(
        SCParser *parser,
        const char *const VAR_NAME
        )
{
    SCItem *item = SCParserSearch(parser, VAR_NAME);
    if(!item)
    {   item = SCParserSearchSlow(parser, VAR_NAME);
    }
    if(item)
    {   
        if(item->allocated)
        {   free(item->name);
        }
        free(item->typename);
        memset(item, 0, sizeof(SCItem));
    }
    return !item;
}

int
SCParserSaveVar(
        SCParser *parser,
        const char *const VAR_NAME,
        void *data
        )
{
    const int FAILURE = 1;
    const int SUCCESS = 0;
    if(!parser || !VAR_NAME)
    {   return FAILURE;
    }


    SCItem *item = SCParserSearch(parser, VAR_NAME);
    if(!item)
    {   item = SCParserSearchSlow(parser, VAR_NAME);
    }
    if(!item)
    {   return FAILURE;
    }

    if(!item->data)
    {   item->data = malloc(item->size);
    }
    if(!item->data)
    {   return FAILURE;
    }
    memcpy(item->data, data, item->size);
    return SUCCESS;
}


