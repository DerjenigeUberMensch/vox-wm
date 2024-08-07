#ifndef _WM_GET_PROP_H
#define _WM_GET_PROP_H

#include "XCB-TRL/xcb_trl.h"
#include "prop.h"
#include "queue.h"


typedef struct PropHandler PropHandler;

struct 
PropHandler
{
    CQueue queue;
    uint32_t use_threads;
    uint32_t thread_length;
    uint32_t thread_index;
    pthread_t *threads;
    GetPropCookie *queue_data;
    uint32_t queue_length;
};


PropHandler *PropCreateStatic(void);
void PropInit(PropHandler *handler);
void PropDestroy(PropHandler *handler);
void PropListen(PropHandler *handler, XCBDisplay *display, XCBWindow win, enum PropertyType type);
void PropListenArg(PropHandler *handler, XCBDisplay *display, XCBWindow win, enum PropertyType type, PropArg arg);


#endif
