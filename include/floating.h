#ifndef __WM__FLOATING__H__
#define __WM__FLOATING__H__

#include <stdint.h>

#include "XCB-TRL/xcb_trl.h"

#include "util.h"

struct Client;

int32_t MAX_ROUNDING_ERROR(void);
uint32_t NonNull DOCKEDVERT(struct Client *c);
uint32_t NonNull DOCKEDHORZ(struct Client *c);
uint32_t NonNull DOCKED(struct Client *c);
uint32_t NonNull ISMAXHORZ(struct Client *c);
uint32_t NonNull ISMAXVERT(struct Client *c);
double NonNull COULDBEFLOATINGGEOM(struct Client *c);
double NonNull COULDBEFLOATINGHINTS(struct Client *c);
bool NonNull SHOULDBEFLOATING(struct Client *c);
bool NonNull SHOULDCENTER(struct Client *c);
uint32_t NonNull SHOULDMAXIMIZE(struct Client *c);
uint32_t NonNull DOCKEDINITIAL(struct Client *c);
uint32_t NonNull WASDOCKEDVERT(struct Client *c);
uint32_t NonNull WASDOCKEDHORZ(struct Client *c);
uint32_t NonNull WASDOCKED(struct Client *c);
uint32_t SHOULDMANAGE(XCBWindow window);
struct Client *NonNull REMOVECLIENTREFERENCES(struct Client *c);
uint32_t NonNull ISFIXED(struct Client *c);
uint32_t NonNull ISURGENT(struct Client *c);
/* flag */
uint32_t NonNull NEVERFOCUS(struct Client *c);
/* client state */
uint32_t NonNull NEVERHOLDFOCUS(struct Client *c);
uint32_t NonNull ISVISIBLE(struct Client *c);

/* 
 */
uint32_t NonNull ISMAPPED(struct Client *c); 
uint32_t NonNull SHOWDECOR(struct Client *c);
uint32_t NonNull ISSELECTED(struct Client *c);

uint16_t NonNull OLDWIDTH(struct Client *c);
uint16_t NonNull OLDHEIGHT(struct Client *c);
uint16_t NonNull WIDTH(struct Client *c);
uint16_t NonNull HEIGHT(struct Client *c);

/* manage */
uint32_t CANMANAGE(XCBWindow win, bool allow_unmapped_window, XCBGetWindowAttributes *waattributes, XCBWindowProperty *wastate);

#endif