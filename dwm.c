

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>

#include <xcb/xcb_atom.h>
#include "xcb_trl.h"
#include "xcb_winutil.h"
#include "util.h"
#include "events.h"
#include "dwm.h"

#include "config.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

int screen;
int running;
int sw;
int sh;
XCBWindow root;
XCBDisplay *dpy;
int numlockmask = 0;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];

void
exithandler(void)
{
    DEBUG("%s", "Process Terminated Successfully.");
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   DEBUG("%s", "An error occured.");
    }
    else
    {   DEBUG("%s", "Nothing occured.");
    }
}


void
argcvhandler(int argc, char *argv[])
{
    int i;
    for(i = 0; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-h"))
        {
            printf( "Usage: dwm [options]\n"
                    "  -h           Help Information.\n"
                    "  -v           Compiler Information.\n"
                    );
            exit(EXIT_SUCCESS);
        }
        else if (!strcmp(argv[i], "-v"))
        {
            char *compiler = "UNKNOWN";
            int majorversion = -1;
            int minorversion = -1;
            int patchversion = -1;
            #if __GNUC__
            compiler = "GCC";
            majorversion = __GNUC__;
            minorversion = __GNUC_MINOR__;
            patchversion = __GNUC_PATCHLEVEL__;
            #endif
            #if __clang__
            compiler = "CLANG";
            majorversion = __clang_major__;
            minorversion = __clang_minor__;
            patchversion = __clang_patchlevel__;
            #endif
            printf( "Compiling Information.\n"
                    "  Compiled:        %s %s\n"
                    "  Compiler:        [%s v%d.%d.%d]\n" 
                    "  STDC:            [%d] [%lu]\n"
                    "  BYTE_ORDER:      [%d]\n"
                    "  POINTER_SIZE:    [%d]\n"
                    "Version Information.\n"
                    "  VERSION:         [%s]\n"
                    , 
                    __DATE__, __TIME__,
                    compiler, majorversion, minorversion, patchversion,
                    __STDC_HOSTED__, __STDC_VERSION__,
                    __BYTE_ORDER__,
                    __SIZEOF_POINTER__,
                    VERSION
                    );
            exit(EXIT_SUCCESS);
        }
        else
        {   
            const char exec1 = '.';
            const char exec2 = '/';
            const char execcount = 3; /* not 2 because we need \0 */ /* +1 for the possible 1 letter name and +1 again for \0   */
            if(argv[0] != NULL && strnlen(argv[0], execcount + 2) > execcount && argv[0][0] == exec1 && argv[0][1] == exec2)
            {   
                if(i > 1)
                {
                    printf("%s%s%s", "UNKNOWN COMMAND: '", argv[i], "'\n");
                    printf( "Usage: dwm [options]\n"
                            "  -h           Help Information.\n"
                            "  -v           Compiler Information.\n"
                          );
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {   
            }
        }
    }
}

void
checkotherwm(void)
{
    XCBGenericEvent *ev = NULL;
    int response;
    XCBSelectInput(dpy, XCBDefaultRootWindow(dpy, screen), XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    XCBSync(dpy);  /* XCBFlush has different behaviour suprisingly, its undesired though */
    /* XCBPollForEvent calls the XServer itself for the event, So if we get a reply then a type of Window manager must be running */
    if((ev = XCBPollForEvent(dpy)))
    {   
        response = ev->response_type;
        free(ev);
        XCBCloseDisplay(dpy);
        if(response == 0) 
        {   DIE("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
        }
        /* UNREACHABLE */
        DIE("%s", "FATAL: UNKNOWN REPONSE_TYPE");
    }
    /* assuming this isnt a bug we received nothing because we pinged the server for a response and got nothing */
    /* The other edge case is if the display just doesnt work, however this is covered at startup() if(!dpy) { DIE(msg); } */
}

void
cleanup(void)
{
}

void
grabbuttons(XCBWindow win, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK};
        XCBUngrabButton(dpy, XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY, win);
		if (!focused)
        {
            XCBGrabButton(dpy, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY, win, 0, BUTTONMASK, 
                    XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC, XCB_NONE, XCB_NONE);
        }
		for (i = 0; i < LENGTH(buttons); i++)
        {
			if (buttons[i].click == ClkClientWin)
            {
				for (j = 0; j < LENGTH(modifiers); j++)
                {
                    XCBGrabButton(dpy, buttons[i].button, 
                            buttons[i].mask | modifiers[j], 
                            win, 0, BUTTONMASK, 
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC, 
                            XCB_NONE, XCB_NONE);
                }
            }
        }
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		u32 i, j, k;
		u32 modifiers[4] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
        XCBKeyCode *keycodes;

        XCBUngrabKey(dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, root);

        for(i = 0; i < LENGTH(keys); ++i)
        {   keycodes = XCBGetKeyCodes(dpy, keys[i].keysym);
        }

        for(j = 0; keycodes[j] != XCB_NO_SYMBOL; ++j)
        {   
            for(k = 0; k < LENGTH(modifiers); ++k)
            {
                XCBGrabKey(dpy, 
                        keycodes[j], keys[i].mod | modifiers[k], 
                        root, 1, 
                        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
            }
        }
        free(keycodes);
    }
}

void 
run(void)
{
    XCBGenericEvent *ev = NULL;
    uint8_t cleanev = 0;
    while(running)
    {
        if(!(ev = XCBPollForQueuedEvent(dpy)))
        {   XCBNextEvent(dpy, &ev);
            if(!ev)     /* Assume server is dead */
            {   running = 0;
                continue;
            }
        }
        /* returns the enum type of the event */
        cleanev = XCB_EVENT_RESPONSE_TYPE(ev);
        if(handler[(cleanev)])
        {   handler[cleanev](ev);
            free(ev);
            ev = NULL;
        }
        switch(cleanev)
        {
            case XCB_KEY_PRESS:
                DEBUG("%s", "XCB_KEY_PRESS");
                break;
            case XCB_KEY_RELEASE:
                DEBUG("%s", "XCB_KEY_RELEASE");
                break;
            case XCB_BUTTON_PRESS:
                DEBUG("%s", "XCB_BUTTON_PRESS");
                break;
            case XCB_BUTTON_RELEASE:
                DEBUG("%s", "XCB_BUTTON_RELEASE");
                break;
            case XCB_MOTION_NOTIFY:
                DEBUG("%s", "XCB_MOTION_NOTIFY");
                break;
            case XCB_ENTER_NOTIFY:
                DEBUG("%s", "XCB_ENTER_NOTIFY");
                break;
            case XCB_LEAVE_NOTIFY:
                DEBUG("%s", "XCB_LEAVE_NOTIFY");
                break;
            case XCB_FOCUS_IN :
                DEBUG("%s", "XCB_FOCUS_IN");
                break;
            case XCB_FOCUS_OUT:
                DEBUG("%s", "XCB_FOCUS_OUT");
                break;
            case XCB_KEYMAP_NOTIFY:
                DEBUG("%s", "XCB_KEYMAP_NOTIFY");
                break;
            case XCB_EXPOSE:
                DEBUG("%s", "XCB_EXPOSE");
                break;
            case XCB_GRAPHICS_EXPOSURE:
                DEBUG("%s", "XCB_GRAPHICS_EXPOSURE");
                break;
            case XCB_NO_EXPOSURE:
                DEBUG("%s", "XCB_NO_EXPOSURE");
                break;
            case XCB_VISIBILITY_NOTIFY:
                DEBUG("%s", "XCB_VISIBILITY_NOTIFY");
                break;
            case XCB_CREATE_NOTIFY:
                DEBUG("%s", "XCB_CREATE_NOTIFY");
                break;
            case XCB_DESTROY_NOTIFY:
                DEBUG("%s", "XCB_DESTROY_NOTIFY");
                break;
            case XCB_UNMAP_NOTIFY:
                DEBUG("%s", "XCB_UNMAP_NOTIFY"); 
                break;
            case XCB_MAP_NOTIFY:
                DEBUG("%s", "XCB_MAP_NOTIFY");
                break;
            case XCB_MAP_REQUEST:
                DEBUG("%s", "XCB_MAP_REQUEST");
                break;
            case XCB_REPARENT_NOTIFY:
                DEBUG("%s", "XCB_REPARENT_NOTIFY");
                break;
            case XCB_CONFIGURE_NOTIFY:
                DEBUG("%s", "XCB_CONFIGURE_NOTIFY");
                break;
            case XCB_CONFIGURE_REQUEST:
                DEBUG("%s", "XCB_CONFIGURE_REQUEST");
                break;
            case XCB_GRAVITY_NOTIFY:
                DEBUG("%s", "XCB_GRAVITY_NOTIFY");
                break;
            case XCB_RESIZE_REQUEST: 
                DEBUG("%s", "XCB_RESIZE_REQUEST");
                break;
            case XCB_CIRCULATE_NOTIFY:
                DEBUG("%s", "XCB_CIRCULATE_NOTIFY");
                break;
            case XCB_CIRCULATE_REQUEST:
                DEBUG("%s", "XCB_CIRCULATE_REQUEST");
                break;
            case XCB_PROPERTY_NOTIFY:
                DEBUG("%s", "XCB_PROPERTY_NOTIFY");
                break;
            case XCB_SELECTION_CLEAR:
                DEBUG("%s", "XCB_SELECTION_CLEAR");
                break;
            case XCB_SELECTION_REQUEST:
                DEBUG("%s", "XCB_SELECTION_REQUEST");
                break;
            case XCB_SELECTION_NOTIFY:
                DEBUG("%s", "XCB_SELECTION_NOTIFY");
                break;
            case XCB_COLORMAP_NOTIFY:
                DEBUG("%s", "XCB_COLORMAP_NOTIFY");
                break;
            case XCB_CLIENT_MESSAGE:
                DEBUG("%s", "XCB_CLIENT_MESSAGE");
                break;
            case XCB_MAPPING_NOTIFY:
                DEBUG("%s", "XCB_MAPPING_NOTIFY");
                break;
            case XCB_GE_GENERIC:
                DEBUG("%s", "XCB_GE_GENERIC");
                break;
            case XCB_NONE:
                DEBUG("%s", "AN ERROR OCCURED");
                break;
            default:
                DEBUG("%s[%d]", "UNKNOWN EVENT CODE: ", cleanev);
        }
    }
}

void
scan(void)
{
}

void
setup(void)
{
    /* globals */
    running = 1;

    /* clean up any zombies immediately */
    sighandler();

    sw = XCBDisplayWidth(dpy, screen);
    sh = XCBDisplayHeight(dpy, screen);
    root = XCBRootWindow(dpy, screen);

    const XCBAtomCookie utf8cookie = XCBInternAtomCookie(dpy, "UTF8_STRING", False);
    XCBInitAtoms(dpy, wmatom, netatom);
    const XCBAtom utf8str = XCBInternAtomReply(dpy, utf8cookie);
                            /* simple window */
    XCBWindow wmcheckwin = XCBCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XCBChangeProperty(dpy, wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&wmcheckwin, 1);
    XCBChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8str, 8, XCB_PROP_MODE_REPLACE, WM_NAME, LENGTH(WM_NAME));
    XCBChangeProperty(dpy, root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(dpy, root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBDeleteProperty(dpy, root, netatom[NetClientList]);

    
    XCBWindowAttributes wa;
    wa.event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|
                    XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_KEY_RELEASE|
                    XCB_EVENT_MASK_KEYMAP_STATE | 
                    XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|
                    XCB_EVENT_MASK_POINTER_MOTION|
                    XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_LEAVE_WINDOW
                    |XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE;
    XCBChangeWindowAttributes(dpy, root, XCB_CW_EVENT_MASK, &wa);
    XCBSelectInput(dpy, root, wa.event_mask);
    grabkeys();
}

void
sigchld(int signo) /* signal */
{
    /* wait for childs (zombie proccess) to die */
    (void)signo;
    while (0 < waitpid(-1, NULL, WNOHANG));
}

void
sighandler(void)
{
    if(signal(SIGCHLD, &sigchld) == SIG_ERR)
    {   DIE("%s", "FATAL: CANNOT_INSTALL_SIGCHLD_HANDLER");
    }
    if(signal(SIGTERM, &sigterm) == SIG_ERR) 
    {   
        DIE("%s", "FATAL: CANNOT_INSTALL_SIGTERM_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    if(signal(SIGHUP, &sighup) == SIG_ERR) 
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGHUP_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    /* wait for zombies to die */
    sigchld(0);
}

void
sighup(int signo) /* signal */
{
    (void)signo;
}

void
sigterm(int signo)
{
    (void)signo;
}

void
startup(void)
{
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    /* XXX rember to set this to NULL when done testing */
    dpy = XCBOpenDisplay(":1", &screen);
    if(!dpy)
    {   DIE("%s", "FATAL: CANNOT_CONNECT_TO_X_SERVER");
    }
    checkotherwm();
    atexit(exithandler);
}

void
updatenumlockmask(void)
{
    /* taken from i3 */
    XCBKeyboardModifier *reply;
    XCBGenericError *err;

    reply = xcb_get_modifier_mapping_reply(dpy, xcb_get_modifier_mapping(dpy), &err);
    if(err)
    {   return;
    }

	xcb_keycode_t *codes = xcb_get_modifier_mapping_keycodes(reply);
	xcb_keycode_t target, *temp = NULL;
	unsigned int i, j;
    static XCBKeySymbols *syms;
    /* init syms before we grab keys */
    syms = xcb_key_symbols_alloc(dpy);
    if(!(temp = xcb_key_symbols_get_keycode(syms, XK_Num_Lock)))
    {   free(reply);
        return;
    }

	target = *temp;
	free(temp);

	for(i = 0; i < 8; i++)
		for(j = 0; j < reply->keycodes_per_modifier; j++)
			if(codes[i * reply->keycodes_per_modifier + j] == target)
				numlockmask = (1 << i);
    free(reply);
}

int
main(int argc, char *argv[])
{
    argcvhandler(argc, argv);
    startup();
    setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
    scan();
    run();
    cleanup();
    XCBCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
