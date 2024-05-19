#ifndef EVENTS_H_
#define EVENTS_H_

#include "XCB-TRL/xcb_trl.h"

/* Handles Key Press Events (Self explanitory) */
void keypress(XCBGenericEvent *event);
/* Handles Key Release Events (Self explanitory) */
void keyrelease(XCBGenericEvent *event);
/* Handles Button Release Events (Self explanitory) */
void buttonpress(XCBGenericEvent *event);
/* Handles Button Release Events (Self explanitory) */
void buttonrelease(XCBGenericEvent *event);
/* Handles Motion Events (AKA mouse movement.) */
void motionnotify(XCBGenericEvent *event);
/* Handles Enter Events (AKA "entering" a client/mouse going inside/ontop a window) */
void enternotify(XCBGenericEvent *event);
/* Handles Leave Events (AKA "leaving" a client/mouse going outside of a window) */
void leavenotify(XCBGenericEvent *event);
/* Handles Focus in Events (AKA when a client gets focused) */
void focusin(XCBGenericEvent *event);
/* Handles Focus out Events (AKA when a client is unfocused) */
void focusout(XCBGenericEvent *event);
/* Handles Keymap events (AKA when a client requests for a certain key "combination" (map) for them)*/
void keymapnotify(XCBGenericEvent *event);
/* Handles Expose events (Not sure.) */
void expose(XCBGenericEvent *event);
/* Handles Graphics Expose Events (Not sure.) */
void graphicsexpose(XCBGenericEvent *event);
/* Handles No Expose Events (Not sure.) */
void noexpose(XCBGenericEvent *event);
/* Handles Circulate Events
 * (This is for the most part just more explicit configure request, With Sibling/Stackmode)
 */
void circulaterequest(XCBGenericEvent *event);
/* Handles Configure Events. 
 * These are events that pertain to the change of a windows geometry, or stackmode.
 */
void configurerequest(XCBGenericEvent *event);
/* Handles Map Events.
 * This event requests to map a window, this request Can be ignored, 
 * Though you wouldnt render anything if you did.
 */
void maprequest(XCBGenericEvent *event);
/* This supposedly is used for clients requesting geometry changes of windows from configurerequest();
 * Though I am not sure what this does exactly/
 */
void resizerequest(XCBGenericEvent *event);
/* Handles Circulate Notify Events.
 * Event reports successfull circulate requests done for a window.
 * (And what they changed/did.)
 */
void circulatenotify(XCBGenericEvent *event);
/* Handles Configure Notify Events.
 * Event reports successfull configure requests done for a window.
 * (And what they changed/did.)
 */
void configurenotify(XCBGenericEvent *event);
/* Handles Window Create Events.
 * Event reports any client requesting for the creation of a new window.
 * Do note that it only reports creation of a Window, it doesnt report Window mapping.
 */
void createnotify(XCBGenericEvent *event);
/* Handles Window Destroy Events.
 * Event reports any destroyed windows.
 */
void destroynotify(XCBGenericEvent *event);
/*
 */
void gravitynotify(XCBGenericEvent *event);
/*
 */
void mapnotify(XCBGenericEvent *event);
/*
 */
void mappingnotify(XCBGenericEvent *event);
/*
 */
void unmapnotify(XCBGenericEvent *event);
/*
 */
void visibilitynotify(XCBGenericEvent *event);
/*
 */
void reparentnotify(XCBGenericEvent *event);
/*
 */
void colormapnotify(XCBGenericEvent *event);
/* Handles Window Message Events.
 * These events inform other windows of stuff they want to change to the Window manager. (For your job anyway.)
 * This is a very broad event and is ussualy interpreted by what atom was sent with.
 */
void clientmessage(XCBGenericEvent *event);
/* Handles Property Notify Events.
 * These events inform windows of changes the XServer made to properties of other windows.
 * This is a very broad event and is ussually interpreted by what atom was sent with.
 */
void propertynotify(XCBGenericEvent *event);
/* Handles Selection Clear Events 
 * Im not sure what this event does.
 */
void selectionclear(XCBGenericEvent *event);
/* Handles Selection Clear Events 
 * Im not sure what this event does.
 */
void selectionnotify(XCBGenericEvent *event);
/* Handles Selection Clear Events 
 * Im not sure what this event does.
 */
void selectionrequest(XCBGenericEvent *event);
/* This event shouldnt be possible, but is here as some "events" sent by clients may be broken/
 * AKA this is just a placeholder. (It does nothing.) 
 */
void genericevent(XCBGenericEvent *event);

void errorhandler(XCBGenericEvent *error);

#endif
