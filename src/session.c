

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#include "main.h"

#include "parser.h"
#include "session.h"
#include "file_util.h"

extern WM _wm;


static int
__SESSION__CREATE__PATH(
        char *buff,
        unsigned int buff_len,
        unsigned int *len_return
        )
{
    /* \0 */
    const int nullbytesize = sizeof(char);
    const unsigned int WM_DIR_NAME_LENGTH = sizeof(_WM_DIR_NAME_) - 1;

    unsigned int len = 0;
    unsigned int usablelen = 0;
    int status;

    status = FFGetSysConfigPath(buff, buff_len - nullbytesize, &len);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    usablelen = buff_len - len;

    if(usablelen - WM_DIR_NAME_LENGTH < 0)
    {   return EXIT_FAILURE;
    }

    strncat(buff, _WM_DIR_NAME_, usablelen);
    usablelen -= WM_DIR_NAME_LENGTH;

    status = FFCreateDir(buff);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    *len_return = buff_len - usablelen;

    return EXIT_SUCCESS;
}

static int
__SESSION__GET__PATH(
        char *buff,
        unsigned int buff_len,
        unsigned int *len_return
        )
{
    return __SESSION__CREATE__PATH(buff, buff_len, len_return);
}

static const inline unsigned int 
__GET__CONFIG__BUFF__SIZE(
        void
        )
{
    /* *Most filenames can only have upto 255 characters */
    const unsigned int MAX_FILENAME = 255;
    /* Assuming /home/user/.config/mydir/dirname/filename
     * x * 2 to allow for upto x2 layers of config directories (incase the user does some wacky stuff)
     */
    const unsigned int PROBABLE_DEPTH = 6;
    const unsigned int MAX_LAYERS = PROBABLE_DEPTH * 2;
    const unsigned int ret = MAX_FILENAME * MAX_LAYERS * sizeof(char);
    return ret;
}


void
SessionSave(
        void
        )
{
    const unsigned int BUFF_SIZE = __GET__CONFIG__BUFF__SIZE();
    char buff[BUFF_SIZE];
}

int
SessionSaveMon(
        SessionMonSave *save,
        char *buff,
        unsigned int buff_len
        )
{
    const u16 deskcount = save->deskcount;
    snprintf(
            buff, buff_len, 
            "DeskCount: %d" "\n"
            "" "\n"
            "" "\n"
            , 
            deskcount
            );
    return EXIT_SUCCESS;
}





void
SessionGetMonSave(
        Monitor *m,
        SessionMonSave *save_return
        )
{
    SessionMonSaveID *id = &save_return->id;

    const i16 mx = m->mx;
    const i16 my = m->my;
    const i16 mw = m->mw;
    const i16 mh = m->mh;

    /* ID */
    id->mx = mx;
    id->my = my;
    id->mw = mw;
    id->mh = mh;

    const u16 deskcount = m->deskcount;
    const u16 deskselnum = m->desksel->num;

    /* settings */
    save_return->deskcount = deskcount;
    save_return->desksel = deskselnum;
}

void
SessionGetDesktopSave(
        Desktop *desk,
        SessionDesktopSave *save_return
        )
{
    SessionDesktopSaveID *id = &save_return->id;

    const i16 num = desk->num;

    /* ID */
    id->num = num;

    const u8 layout = desk->layout;
    const u8 olayout = desk->olayout;
    const XCBWindow sel = desk->sel ? desk->sel->win : 0;

    /* settings */

    save_return->layout = layout;
    save_return->olayout = olayout;
    save_return->sel = sel;
}

void
SessionGetClientSave(
        Client *c,
        SessionClientSave *save_return
        )
{
    SessionClientSaveID *id = &save_return->id;

    const XCBWindow win = c->win;
    /* ID */
    id->win = win;

    const i16 x = c->x;
    const i16 y = c->y;
    const u16 w = c->w;
    const u16 h = c->h;

    const i16 ox = c->oldx;
    const i16 oy = c->oldy;
    const i16 ow = c->oldw;
    const i16 oh = c->oldh;

    const u16 bw = c->bw;
    const u16 obw = c->oldbw;
    const u32 bcol = c->bcol;

    const float mina = c->mina;
    const float maxa = c->maxa;

    const u16 basew = c->basew;
    const u16 baseh = c->baseh;

    const u16 incw = c->incw;
    const u16 inch = c->inch;

    const u16 maxw = c->maxw;
    const u16 maxh = c->maxh;
    
    const u16 minw = c->minw;
    const u16 minh = c->minh;

    const pid_t pid = c->pid;

    const u16 rstacknum = c->rstacknum;
    const u16 flags = c->flags;
    const u32 ewmhflags = c->ewmhflags;
    const enum XCBBitGravity gravity = c->gravity;


    /* settings */

    save_return->x = x;
    save_return->y = y;
    save_return->w = w;
    save_return->h = h;

    save_return->ox = ox;
    save_return->oy = oy;
    save_return->ow = ow;
    save_return->oh = oh;

    save_return->bw = bw;
    save_return->obw = obw;
    save_return->bcol = bcol;

    save_return->mina = mina;
    save_return->maxa = maxa;

    save_return->basew = basew;
    save_return->baseh = baseh;

    save_return->incw = incw;
    save_return->inch = inch;

    save_return->maxw = maxw;
    save_return->maxh = maxh;

    save_return->minw = minw;
    save_return->minh = minh;

    save_return->pid = pid;

    save_return->rstacknum = rstacknum;
    save_return->flags = flags;
    save_return->ewmhflags = ewmhflags;
    save_return->gravity = gravity;
}


