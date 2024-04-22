#include "config.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>




void
__attach(CFGSave *cf, CFGSaveItem *i)
{
    i->next = cf->items;
    cf->items = i;
    if(i->next)
    {   i->next->prev = i;
    }
    else
    {
        /*
        cf->last = i;
        */
    }
    i->prev = NULL;
}

void
__detach(CFGSave *cf, CFGSaveItem *i)
{
    CFGSaveItem **tc;
    for(tc = &cf->items; *tc && *tc != i; tc = &(*tc)->next);
    *tc = i->next;
    if(!(*tc))
    {
        /*
        cf->last = i->prev;
        */
    }
    else if(i->next)
    {
        i->next->prev = i->prev;
    }
    else if(i->prev)
    {
        /*
         * cf->last = i->prev;
         */
        i->prev->next = NULL;
    }
}


CFGSave *
CFGCreateSave(const char *file_name, uint8_t human_redeable)
{
    CFGSave *cs = malloc(sizeof(CFGSave));

    if(cs)
    {
        cs->items = 0;
        cs->file = (char *)file_name;
        cs->redeable = human_redeable;
    }

    return cs;
}

uint8_t 
CFGCreateVar(CFGSave *cfgsf, const char *var_name, uint8_t _type)
{
    CFGSaveItem *item = calloc(1, sizeof(CFGSaveItem));

    if(item)
    {
        item->_type = _type;
        item->name = (char *)var_name;
        item->data = NULL;
        __attach(cfgsf, item);
    }
    return !!item;
}

uint8_t 
CFGSaveVar(CFGSave *cfgsf, const char *save, void *data)
{
    CFGSaveItem *item;
    for(item = cfgsf->items; item; item = item->next)
    {
        if(!strcmp(save, item->name))
        {   item->data = data;
            return 1;
        }
    }
    return 0;
}

uint8_t
CFGWrite(CFGSave *cfgsf)
{
    CFGSaveItem *item;
    char *format;
    FILE *fw = fopen(cfgsf->file, "w");
    uint8_t error = 0;
    if(!fw)
    {   return 0;
    }
    for(item = cfgsf->items; item; item = item->next)
    {
        switch(item->_type)
        {
            case INTEGER:
                format = "%s%d";
                break;
            case FLOAT:
                format = "%s%f";
                break;
            case DOUBLE:
                format = "%s%f";
                break;
            case CHAR:
                format = "%s%c";
                break;
            case STRING:
                format = "%s%s";
                break;
            default:
                error += !error;
                continue;
        }
        fprintf(fw, format, item->name, item->data);
    }
    return !error;
}

uint8_t
CFGRead(CFGSave *cfgsf)
{
    CFGSaveItem *item;
    FILE *file = fopen(cfgsf->file, "r");
    for(item = cfgsf->items; item; item = item->next)
    {
        if(!strcmp(save, item->name))
        {   item->data = data;
            return 1;
        }
    }
    return 0;
}
