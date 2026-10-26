#ifndef EVENTS_H_
#define EVENTS_H_

#include "XCB-TRL/xcb_trl.h"
#include "util.h"

/* Handles Key Press Events (Self explanitory) */
void NonNull keypress(XCBEvent *event);
/* Handles Key Release Events (Self explanitory) */
void NonNull keyrelease(XCBEvent *event);
/* Handles Button Release Events (Self explanitory) */
void NonNull buttonpress(XCBEvent *event);
/* Handles Button Release Events (Self explanitory) */
void NonNull buttonrelease(XCBEvent *event);
/* Handles Motion Events (AKA mouse movement.) */
void NonNull motionnotify(XCBEvent *event);
/* Handles Enter Events (AKA "entering" a client/mouse going inside/ontop a window) */
void NonNull enternotify(XCBEvent *event);
/* Handles Leave Events (AKA "leaving" a client/mouse going outside of a window) */
void NonNull leavenotify(XCBEvent *event);
/* Handles Focus in Events (AKA when a client gets focused) */
void NonNull focusin(XCBEvent *event);
/* Handles Focus out Events (AKA when a client is unfocused) */
void NonNull focusout(XCBEvent *event);
/* Handles Keymap events (AKA when a client requests for a certain key "combination" (map) for them)*/
void NonNull keymapnotify(XCBEvent *event);
/* Handles Expose events (Not sure.) */
void NonNull expose(XCBEvent *event);
/* Handles Graphics Expose Events (Not sure.) */
void NonNull graphicsexpose(XCBEvent *event);
/* Handles No Expose Events (Not sure.) */
void NonNull noexpose(XCBEvent *event);
/* Handles Circulate Events
 * (This is for the most part just more explicit configure request, With Sibling/Stackmode)
 */
void NonNull circulaterequest(XCBEvent *event);
/* Handles Configure Events.
 * These are events that pertain to the change of a windows geometry, or stackmode.
 */
void NonNull configurerequest(XCBEvent *event);
/* Handles Map Events.
 * This event requests to map a window, this request Can be ignored,
 * Though you wouldnt render anything if you did.
 */
void NonNull maprequest(XCBEvent *event);
/* This supposedly is used for clients requesting geometry changes of windows from configurerequest();
 * Though I am not sure what this does exactly/
 */
void NonNull resizerequest(XCBEvent *event);
/* Handles Circulate Notify Events.
 * Event reports successfull circulate requests done for a window.
 * (And what they changed/did.)
 */
void NonNull circulatenotify(XCBEvent *event);
/* Handles Configure Notify Events.
 * Event reports successfull configure requests done for a window.
 * (And what they changed/did.)
 */
void NonNull configurenotify(XCBEvent *event);
/* Handles Window Create Events.
 * Event reports any client requesting for the creation of a new window.
 * Do note that it only reports creation of a Window, it doesnt report Window mapping.
 */
void NonNull createnotify(XCBEvent *event);
/* Handles Window Destroy Events.
 * Event reports any destroyed windows.
 */
void NonNull destroynotify(XCBEvent *event);
/*
 */
void NonNull gravitynotify(XCBEvent *event);
/*
 */
void NonNull mapnotify(XCBEvent *event);
/*
 */
void NonNull mappingnotify(XCBEvent *event);
/*
 */
void NonNull unmapnotify(XCBEvent *event);
/*
 */
void NonNull visibilitynotify(XCBEvent *event);
/*
 */
void NonNull reparentnotify(XCBEvent *event);
/*
 */
void NonNull colormapnotify(XCBEvent *event);
/* Handles Window Message Events.
 * These events inform other windows of stuff they want to change to the Window manager. (For your job anyway.)
 * This is a very broad event and is ussualy interpreted by what atom was sent with.
 */
void NonNull clientmessage(XCBEvent *event);
/* Handles Property Notify Events.
 * These events inform windows of changes the XServer made to properties of other windows.
 * This is a very broad event and is ussually interpreted by what atom was sent with.
 */
void NonNull propertynotify(XCBEvent *event);
/* Handles Selection Clear Events
 * Im not sure what this event does.
 */
void NonNull selectionclear(XCBEvent *event);
/* Handles Selection Clear Events
 * Im not sure what this event does.
 */
void NonNull selectionnotify(XCBEvent *event);
/* Handles Selection Clear Events
 * Im not sure what this event does.
 */
void NonNull selectionrequest(XCBEvent *event);
/* This event shouldnt be possible, but is here as some "events" sent by clients may be broken/
 * AKA this is just a placeholder. (It does nothing.)
 */
void NonNull genericevent(XCBEvent *event);

void NonNull errorhandler(XCBEvent *error);

#endif
