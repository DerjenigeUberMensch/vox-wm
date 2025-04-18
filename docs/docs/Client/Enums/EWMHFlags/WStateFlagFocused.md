# WStateFlagFocused

Indicates that a client is focused.

This is a read-only hint for clients.  
This is set by the window manager.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_STATE_FOCUSED` indicates whether the window's decorations are drawn in an active state.


Clients MUST regard it as a read-only hint. 

It cannot be set at map time or changed via a _NET_WM_STATE client message.
The window given by _NET_ACTIVE_WINDOW will usually have this hint, but at times other windows may as well, 
if they have a strong association with the active window and will be considered as a unit with it by the user. 
Clients that modify the appearance of internal elements when a toplevel has keyboard focus SHOULD check for the availability of this state in _NET_SUPPORTED and, if it is available, use it in preference to tracking focus via FocusIn events. 
By doing so they will match the window decorations and accurately reflect the intentions of the Window Manager.   

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagDemandAttention = 1u << 26,
    ...,
};
```
