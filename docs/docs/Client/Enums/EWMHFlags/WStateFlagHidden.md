# WStateFlagHidden

Indicates that a client is not in any way shown currently.


It is recommened that hidden clients are unmapped, however is not required.  
This is set by the window manager.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_STATE_HIDDEN` set by the Window Manager to indicate that a window would not be visible on the screen if its desktop/viewport were active and its coordinates were within the screen bounds.

The canonical example is that minimized windows should be in the _NET_WM_STATE_HIDDEN state. Pagers and similar applications should use _NET_WM_STATE_HIDDEN instead of WM_STATE to decide whether to display a window in miniature representations of the windows on a desktop. 

Implementation note: if an Application asks to toggle _NET_WM_STATE_HIDDEN the Window Manager should probably just ignore the request, since _NET_WM_STATE_HIDDEN is a function of some other aspect of the window such as minimization, rather than an independent state. 

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagHidden = 1u << 21,
    ...,
};
```
