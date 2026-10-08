#ifndef __KEYBIND_EXTRAS_H__
#define __KEYBIND_EXTRAS_H__

#include "main.h"

enum WMKeyType;

/* Handles keybinds and calls the appropriate function if a keybind is found.
 *
 * RETURN: true if a keybind was found and handled.
 * RETURN: false if no keybind was found or if an error occurred.
 */
bool WMKeybindHandler(u16 mod, XCBKeyCode code, enum WMKeyType type, bool is_button);
/* Try and find the keysym from the string provided.
 *
 * RETURN: XCBKeysym on Succesful string to keybind lookup.
 * RETURN: XCB_NON_SYMBOL on Failure.
 */
XCBKeysym WMKeybindKeysymFromString(const char *keysym_str);

/* Try and find the modifier from the string provided as a XCBModifier (XCBModMask1/XCBModMask2/etc...).
 *
 * RETURN: uint16_t on Succesful string to modifier lookup.
 * RETURN: 0 on Failure.
 */
uint16_t WMKeybindModifierFromString(const char *mod_str);

#endif