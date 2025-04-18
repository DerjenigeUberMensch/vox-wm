# WTypeFlagDialog

Indicates that a client is a dialog box/window.

These windows should have the middle ground priority within the window manager.

However should have slightly lower priority compared to modal dialog boxes,
and but should be roughly the same priority with popup menus.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_DIALOG` indicates that this is a dialog window.

If _NET_WM_WINDOW_TYPE is not set, then managed windows with WM_TRANSIENT_FOR set MUST be taken as this type. 

Override-redirect windows with WM_TRANSIENT_FOR, but without _NET_WM_WINDOW_TYPE must be taken as _NET_WM_WINDOW_TYPE_NORMAL.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagDialog = 1u << 6,
    ...,
};
```
