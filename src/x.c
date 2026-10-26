#include <stdio.h>
#include <string.h>

#include "x.h"

#include "util.h"


#include "XCB-TRL/xcb_winutil.h"

extern XCBAtom netatom[];
extern XCBAtom wmatom[];
extern XCBAtom motifatom;

uint8_t
checksticky(int64_t x)
{
    /* _NET_WM_DESKTOP
     * https://specifications.freedesktop.org/wm-spec/latest/
     */
    return (x == 0xFFFFFFFF);
}

char *
getnamefromreply(XCBWindowProperty *namerep)
{
    char *nstr = NULL;
    if(namerep && (namerep->type == XCB_ATOM_STRING || namerep->type == netatom[NetUtf8String]))
    {
        /* debug against stack smashing */
        /*
        Debug("Length:  [%u]", namerep->length);
        Debug("Type:    [%u]", namerep->type);
        Debug("Format:  [%u]", namerep->format);
        Debug("Sequence:[%u]", namerep->sequence);
        Debug("VALENGTH:[%u]", namerep->value_len);
        Debug("BytesAft:[%u]", namerep->bytes_after);
        Debug("Resptype:[%u]", namerep->response_type);
        */
        uint32_t offset = 0;

        XCBGetPropertyValueSize(namerep, &offset);
        char *str = XCBGetPropertyValue(namerep);
        nstr = malloc(sizeof(char) * offset + sizeof(char));
        if(nstr)
        {   
            if(offset)
            {   memcpy(nstr, str, offset);
            }
            nstr[offset] = '\0';
        }
    }
    return nstr;
}

uint32_t *
geticonprop(XCBWindowProperty *iconreply)
{
    /* reject invalid replies */
    if(!iconreply || iconreply->type != XCB_ATOM_CARDINAL || iconreply->format != 32 || iconreply->length == 0)
    {   return NULL;
    }

    const u32 *icons = XCBGetPropertyValue(iconreply);
    u32 length = 0;
    XCBGetPropertyValueLength(iconreply, sizeof(uint32_t), &length);

    size_t offset = 0;
    size_t best = 0;
    u64 bestarea = 0;

    while(length - offset >= 2)
    {
        u64 area = (u64)icons[offset] * (u64)icons[offset + 1];

        if(!area || area > length - offset - 2)
        {   break;
        }

        if(area > bestarea)
        {
            best = offset;
            bestarea = area;
        }

        offset += (size_t)area + 2;
    }

    if(!bestarea || bestarea > SIZE_MAX / sizeof(u32) - 2)
    {   return NULL;
    }

    size_t bytes = ((size_t)bestarea + 2) * sizeof(u32);

    u32 *ret = malloc(bytes);

    if(ret)
    {   memcpy(ret, icons + best, bytes);
    }

    return ret;
}



void
XCBSetAtomState(XCBDisplay *display, XCBWindow win, XCBAtom type, XCBAtom atom, XCBWindowProperty *prop, u8 _delete)
{
    XCBAtom *atoms = NULL;
    u32 count = 0;
    
    if(prop && prop->type == XCB_ATOM_ATOM && prop->format == 32)
    {   
        atoms = XCBGetPropertyValue(prop);
        XCBGetPropertyValueLength(prop, sizeof(XCBAtom), &count);
    }

    u32 i;
    u32 kept = 0;

    bool found = false;

    for(i = 0; i < count; ++i)
    {
        if(atoms[i] == atom)
        {
            found = true;
        }
        else if(_delete)
        {   
            atoms[kept] = atoms[i];
            ++kept;
        }
    }

    if(_delete)
    {
        if(found)
        {   XCBChangeProperty(display, win, type, XCB_ATOM_ATOM, 32, XCBPropModeReplace, atoms, kept);
        }
    }
    else if(!found)
    {   XCBChangeProperty(display, win, type, XCB_ATOM_ATOM, 32, count ? XCBPropModeAppend : XCBPropModeReplace, &atom, 1);
    }
}

char *
GetAtomNameQuick(XCBDisplay *display, XCBAtom atom)
{
    XCBCookie cookie;
    XCBAtomName *rep;
    cookie = XCBGetAtomNameCookie(display, atom);
    rep = XCBGetAtomNameReply(display, cookie);
    Debug("%u", atom);
    char *buff = NULL;
    if(rep)
    {
        const u32 len = xcb_get_atom_name_name_length(rep);
        const size_t size = sizeof(char) * len + sizeof(char);
        if(len)
        {   buff = malloc(size);
        }
        if(buff)
        {   
            memcpy(buff, xcb_get_atom_name_name(rep), len);
            buff[len] = '\0';
        }
        free(rep);
    }
    return buff;
}
