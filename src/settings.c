#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "settings.h"
#include "util.h"

struct 
USSettingData
{
    const char *const names[UserSettingsLAST];
    const u8 name_len[UserSettingsLAST];
    const u8 field_size[UserSettingsLAST];
    const u16 field_offset[UserSettingsLAST];
    const enum SCType type[UserSettingsLAST];
};

#ifdef ADD_MEMBER
#define ADD_MEMBER1456 ADD_MEMBER
#undef ADD_MEMBER
#endif

#define ADD_MEMBER(NAME, STRUCT_NAME, TYPE) \
        .names[NAME] = #NAME, \
        .name_len[NAME] = sizeof(#NAME), \
        .field_size[NAME] = FIELD_SIZEOF(UserSettings, STRUCT_NAME), \
        .field_offset[NAME] = offsetof(UserSettings, STRUCT_NAME), \
        .type[NAME] = TYPE \
        , \

static const struct USSettingData
__USER__SETTINGS__DATA__ = 
{
    ADD_MEMBER(MFact, mfact, SCTypeFLOAT)
    ADD_MEMBER(GapRatio, gapratio, SCTypeFLOAT)
    ADD_MEMBER(MCount, mcount, SCTypeUSHORT)
    ADD_MEMBER(Snap, snap, SCTypeUSHORT)
    ADD_MEMBER(RefreshRate, refreshrate, SCTypeUSHORT)
    ADD_MEMBER(MaxCC, maxcc, SCTypeUSHORT)

    /* BOOL Types */
    ADD_MEMBER(HoverFocus, hoverfocus, SCTypeBOOL)
    ADD_MEMBER(UseDecorations, usedecorations, SCTypeBOOL)
    ADD_MEMBER(UseClientSideDecorations, useclientdecorations, SCTypeBOOL)
    ADD_MEMBER(PreferClientSideDecorations, preferclientdecorations, SCTypeBOOL)


    /* bar data */
    ADD_MEMBER(BarLX, lx, SCTypeFLOAT)
    ADD_MEMBER(BarLY, ly, SCTypeFLOAT)
    ADD_MEMBER(BarLW, lw, SCTypeFLOAT)
    ADD_MEMBER(BarLH, lh, SCTypeFLOAT)

    ADD_MEMBER(BarRX, rx, SCTypeFLOAT)
    ADD_MEMBER(BarRY, ry, SCTypeFLOAT)
    ADD_MEMBER(BarRW, rw, SCTypeFLOAT)
    ADD_MEMBER(BarRH, rh, SCTypeFLOAT)

    ADD_MEMBER(BarTX, tx, SCTypeFLOAT)
    ADD_MEMBER(BarTY, ty, SCTypeFLOAT)
    ADD_MEMBER(BarTW, tw, SCTypeFLOAT)
    ADD_MEMBER(BarTH, th, SCTypeFLOAT)

    ADD_MEMBER(BarBX, bx, SCTypeFLOAT)
    ADD_MEMBER(BarBY, by, SCTypeFLOAT)
    ADD_MEMBER(BarBW, bw, SCTypeFLOAT)
    ADD_MEMBER(BarBH, bh, SCTypeFLOAT)
};


#undef ADD_MEMBER

#ifdef ADD_MEMBER1456
#define ADD_MEMBER ADD_MEMBER1456
#undef ADD_MEMBER1456
#endif



static char *__CONFIG__NAME__ = NULL;


static int 
USGetConfigPath(char *buff, unsigned int buff_len)
{
    char *home = NULL;
    home = getenv("XDG_CONFIG_HOME");
    if(!home)
    {   home = getenv("HOME");
    }
    if(home)
    {   
        snprintf(buff, buff_len, "%s/.config", home);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/* 
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FAILURE on Failure.
 */
static int
USCreateDir(
        const char *const DIR_NAME
        )
{
    const int NOT_FOUND = -1;
    int statstatus = 0; 
    struct stat st = {0};

    statstatus = stat(DIR_NAME, &st);

    if(statstatus == NOT_FOUND)
    {   
        int mkdirstatus = mkdir(DIR_NAME, 0777);
        if(mkdirstatus && errno != EEXIST)
        {   return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

void
USSetupCFGVars(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    SCParser *cfg = us->cfg;

    i32 i;
    u8 err = 0;
    const int READONLY = 1;
    const struct USSettingData *const usdata = &__USER__SETTINGS__DATA__;
    /* global settings */
    for(i = 0; i < UserSettingsLAST; ++i)
    {   
        err = SCParserNewVar(cfg, usdata->names[i], usdata->name_len[i], READONLY, usdata->field_size[i], usdata->type[i]);
        if(err)
        {   Debug("Failed to create: \"%s\"", usdata->names[i]);
        }
    }
}

void
USSetupCFGDefaults(
        UserSettings *us
        )
{
    if(!us || !us->cfg)
    {   return;
    }

    /* Do note these settings are mostly arbitrary numbers that I (the creator) like */
    UserSettings *s = us;
    const u16 nmaster = 1;

    const u16 refreshrate = 60;
    const float bgw = 0.95f;
    const u16 winsnap = 10;
    const u16 maxcc = 256;      /* Xorg default is 255 or 256, I dont remember */
    const float mfact = 0.55f;

    /* These are bools, due to possible difference in compiler implementation they are u8's */
    const u8 hoverfocus = 0;
    const u8 usedecor = 0;
    const u8 clientdecor = 1;
    const u8 preferclientdecor = 1;

    s->mcount = nmaster;
    s->mfact = mfact;
    s->refreshrate = refreshrate;
    s->gapratio = bgw;
    s->snap = winsnap;
    s->maxcc = maxcc;

    s->hoverfocus = hoverfocus;
    s->usedecorations = usedecor;
    s->useclientdecorations = clientdecor;
    s->preferclientdecorations = preferclientdecor;

    /* Left Stuff */
    s->lw = 0.15f;
    s->lh = 1.0f;
    s->lx = 0.0f;
    s->ly = 0.0f;
    /* Right Stuff */
    s->rw = 0.15f;
    s->rh = 1.0f;
    s->rx = 1.0f - s->rw;
    s->ry = 0.0f;
    /* Top Stuff */
    s->tw = 1.0f;
    s->th = 0.15f;
    s->tx = 0.0f;
    s->ty = 0.0f;
    /* Bottom Stuff */
    s->bw = 1.0f;
    s->bh = 0.15f;
    s->bx = 0.0f;
    s->by = 1.0f - s->bh;
}

void
USInit(
        UserSettings *settings_init
        )
{
    if(!settings_init)
    {   return;
    }
    if(USInitFile() == EXIT_FAILURE)
    {   
        Debug0("Failed to load file");
        return;
    }
    memset(settings_init, 0, sizeof(UserSettings));
    settings_init->cfg = SCParserCreate(UserSettingsLAST);
    if(settings_init->cfg)
    {
        USSetupCFGVars(settings_init);
        USSetupCFGDefaults(settings_init);
        USLoad(settings_init);
    }
}

int
USInitFile(void)
{
#define _US_INIT_DIR_NAME "/vox-wm"
#define _US_INIT_FILE_NAME "/vox.cfg"

    const int HOME_SIZE = 1024;
    const int NAME_SIZE = HOME_SIZE + sizeof(_US_INIT_DIR_NAME) + sizeof(_US_INIT_FILE_NAME);
    char home[HOME_SIZE];
    char filename[NAME_SIZE];

    memset(home, 0, HOME_SIZE);
    memset(filename, 0, NAME_SIZE);

    if(USGetConfigPath(home, HOME_SIZE - 1) == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    strcpy(filename, home);
    strcat(filename, _US_INIT_DIR_NAME);

    if(USCreateDir(filename) == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    strcat(filename, _US_INIT_FILE_NAME);

    u32 len = strlen(filename) + 1;

    __CONFIG__NAME__ = malloc(len);
    if(__CONFIG__NAME__)
    {   memcpy(__CONFIG__NAME__, filename, len);
    }
#undef _US_INIT_FILE_NAME
#undef _US_INIT_DIR_NAME
    if(__CONFIG__NAME__)
    {   return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void
USLoad(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }

    SCParser *cfg = settings->cfg;
    SCItem *item;
    u8 status = SCParserReadFile(cfg, __CONFIG__NAME__);
    if(status)
    {   
        Debug0("Failed to load previous data, loading defaults...");
        return;
    }
    i32 i;
    void *data;
    const struct USSettingData *const usdata = &__USER__SETTINGS__DATA__;
    for(i = 0; i < UserSettingsLAST; ++i)
    {
        data = ((uint8_t *)settings) + usdata->field_offset[i];
        item = SCParserSearch(cfg, usdata->names[i]);
        if(!item)
        {   item = SCParserSearchSlow(cfg, usdata->names[i]);
        }
        if(item)
        {   
            status = SCParserLoad(item, data, usdata->field_size[i], usdata->type[i]);
            if(status)
            {   Debug("Failed to LOAD, \"%s\"", usdata->names[i]);
            }
        }
        else
        {   Debug("Failed to FIND, \"%s\"", usdata->names[i]);
        }
    }
}

void
USSave(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }
    SCParser *cfg = settings->cfg;
    UserSettings *s = settings;
    i32 i;
    const struct USSettingData *const usdata = &__USER__SETTINGS__DATA__;
    for(i = 0; i < UserSettingsLAST; ++i)
    {   SCParserSaveVar(cfg, usdata->names[i], ((uint8_t *)s) + usdata->field_offset[i]);
    }
    FILE *fp = fopen(__CONFIG__NAME__, "r");
    if(fp)
    {   
        fseek(fp, 0, SEEK_END);
        unsigned long int len = ftell(fp);
        fclose(fp);
        /* Only overwrite file if empty */
        if(len == 0)
        {   
            Debug0("Empty file found, writing base config...");
            SCParserWrite(cfg, __CONFIG__NAME__);
        }
    }
    else
    {   
        Debug0("No file found, writing base config...");
        SCParserWrite(cfg, __CONFIG__NAME__);
    }
}

void
USWipe(
        UserSettings *settings
        )
{
    if(!settings || !settings->cfg)
    {   return;
    }
    SCParser *cfg = settings->cfg;
    SCParserDestroy(cfg);
    memset(settings, 0, sizeof(UserSettings));
    free(__CONFIG__NAME__);
}

