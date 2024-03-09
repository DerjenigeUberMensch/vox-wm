#ifndef EVENTS_H_
#define EVENTS_H_

#include "xcb_trl.h"

void keypress(XCBGenericEvent *event);
void keyrelease(XCBGenericEvent *event);
void buttonpress(XCBGenericEvent *event);
void buttonrelease(XCBGenericEvent *event);
void motionnotify(XCBGenericEvent *event);
void enternotify(XCBGenericEvent *event);
void leavenotify(XCBGenericEvent *event);
void focusin(XCBGenericEvent *event);
void focusout(XCBGenericEvent *event);
void keymapnotify(XCBGenericEvent *event);
void expose(XCBGenericEvent *event);
void graphicsexpose(XCBGenericEvent *event);
void noexpose(XCBGenericEvent *event);
void circulaterequest(XCBGenericEvent *event);
void configurerequest(XCBGenericEvent *event);
void maprequest(XCBGenericEvent *event);
void resizerequest(XCBGenericEvent *event);
void circulatenotify(XCBGenericEvent *event);
void configurenotify(XCBGenericEvent *event);
void createnotify(XCBGenericEvent *event);
void destroynotify(XCBGenericEvent *event);
void gravitynotify(XCBGenericEvent *event);
void mapnotify(XCBGenericEvent *event);
void mappingnotify(XCBGenericEvent *event);
void unmapnotify(XCBGenericEvent *event);
void visibilitynotify(XCBGenericEvent *event);
void reparentnotify(XCBGenericEvent *event);
void colormapnotify(XCBGenericEvent *event);
void clientmessage(XCBGenericEvent *event);
void propertynotify(XCBGenericEvent *event);
void selectionclear(XCBGenericEvent *event);
void selectionnotify(XCBGenericEvent *event);
void selectionrequest(XCBGenericEvent *event);
void genericevent(XCBGenericEvent *event);

void errorhandler(XCBGenericEvent *error);

#ifndef LASTEvent
#define LASTEvent   40      /* XCB has a max event count of roughly ~35 however that is a estimate which is why 50 is used (just in case) .
                             * The reason it isnt 33 like in xlib is cause xcb skips ~2 before assigning event opcodes.
                             */
#endif
#endif
