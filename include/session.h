#ifndef __SESSION__H__
#define __SESSION__H__


#include <stddef.h>

#include "XCB-TRL/xcb_trl.h"
#include "settings.h"
#include "parser.h"

#define _WM_DIR_NAME_ "vox-wm"


typedef struct SessionMonSaveID SessionMonSaveID;
typedef struct SessionMonSave SessionMonSave;
typedef struct SessionBarSave SessionBarSave;
typedef struct SessionBarSaveID SessionBarSaveID;

typedef struct SessionDesktopSaveID SessionDesktopSaveID;
typedef struct SessionDesktopSave SessionDesktopSave;

typedef struct SessionClientSaveID SessionClientSaveID;
typedef struct SessionClientSave SessionClientSave;



struct 
SessionMonSaveID
{
    int16_t mx;
    int16_t my;
    uint16_t mw;
    uint16_t mh;
};

struct
SessionBarSaveID
{
    char *classname;
    char *instancename;
};

struct
SessionBarSave
{
    BarSettings bars;
};


struct 
SessionMonSave
{
    uint16_t deskcount;
    uint16_t desksel;
    /* ID */
    SessionMonSaveID id;
};

struct
SessionDesktopSaveID
{   
    int16_t num;
};

struct
SessionDesktopSave
{
    uint8_t layout;
    uint8_t olayout;
    XCBWindow sel;
    /* ID */
    SessionDesktopSaveID id;
};

struct
SessionClientSaveID
{   
    XCBWindow win;
};

struct
SessionClientSave
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    int16_t ox;
    int16_t oy;
    uint16_t ow;
    uint16_t oh;

    uint16_t bw;
    uint16_t obw;
    uint16_t bcol;
    

    float mina;
    float maxa;

    uint16_t basew;
    uint16_t baseh;

    uint16_t incw;
    uint16_t inch;

    uint16_t maxw;
    uint16_t maxh;

    uint16_t minw;
    uint16_t minh;

    pid_t pid;

    uint16_t rstacknum;
    uint16_t flags;
    uint32_t ewmhflags;
    enum XCBBitGravity gravity;
    /* ID */
    SessionClientSaveID id;
};



#endif
