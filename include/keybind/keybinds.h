#ifndef __WM__KEYBINDS__H__
#define __WM__KEYBINDS__H__

#include <stdint.h>

#include <X11/keysym.h>
#include <X11/XF86keysym.h> 

#include "keybind_definitions.h"
#include "keybind_extras.h"
#include "util.h"
#include "main.h"

typedef struct WMKey WMKey;
typedef struct WMButton WMButton;
typedef struct WMKeyBind WMKeyBind;

struct
WMKey
{
    XCBKeysym sym;
    uint8_t pressed;
    uint8_t pad0[3];
};

struct
WMButton
{
    XCBButton button;
    uint8_t pressed;
    uint8_t pad0[3];
};


struct 
WMKeyBind
{
    uint16_t mod;              /* Modifier(s)                      */

    uint8_t type;              /* Press or Release                 */
    uint8_t pad0[5];

    GArray keysyms;             /* sizeof(WMKey)                   */
    GArray buttons;             /* sizeof(WMButton)                */
    Generic arg;                /* Argument                        */

    void (*func)(const WMKeyBind *self, const Generic *arg);  /* Function            */
};


/*
 * Initialize the keybind system
 *
 * RETURN: EXIT_SUCCESS on Success
 * RETURN: EXIT_FAILURE on Failure
 */
int WMKeybindInit(void);
/*
 * Create a new keybind
 *
 * NOTE: Adding or removing a keybind only updates the internal state, please run WMKeybindGrabKeys() after adding or removing keybinds to update the X11 state.
 *
 * RETURN: 0 on Success
 * RETURN: 1 on Failure
 * RETURN: -1 Already Exists
 */
int WMKeybindAdd(uint16_t xcb_modmask_modifiers, XCBKeysym keysyms[], size_t num_keysyms, XCBButton buttons[], size_t num_buttons, void (*func)(const WMKeyBind *self, const Generic *arg), Generic arg, enum WMKeyType type);
/* Remove a keybind
 *
 * NOTE: Adding or removing a keybind only updates the internal state, please run WMKeybindGrabKeys() after adding or removing keybinds to update the X11 state.
 * RETURN: EXIT_SUCCESS on Success
 * RETURN: EXIT_FAILURE on Failure
 */
int WMKeybindRemove(uint16_t modifier_mask_x11, XCBKeysym keysyms[], size_t num_keysyms, XCBButton buttons[], size_t num_buttons);
/* Removes all keybinds. 
 *
 * NOTE: Adding or removing a keybind only updates the internal state, please run WMKeybindGrabKeys() after adding or removing keybinds to update the X11 state.
 *
 * This function does not return a value.
 */
void WMKeybindRemoveAll(void);
/* Destroy the keybind system and free all memory
 * 
 * This function does not return a value.
 */
void WMKeybindDestroy(void);
/* Grab all keys in the keybind system and register them with X11
 *
 * This function does not return a value.
 */
void WMKeybindGrabKeys(void);
/* Grab all buttons for a client and register them with X11
 *
 * This function does not return a value.
 */
void WMKeybindGrabButtons(Client *c, int focused);

#endif