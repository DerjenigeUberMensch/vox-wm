# WTypeFlagMenu

Indicates that a client is a menu (torn off from the main application).

This should be used to detect whether a window should be grouped within an application, and kept above other windows and treated as a dialog box.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_MENU`  indicates pinnable menu windows.

Windows of this type may set the WM_TRANSIENT_FOR hint indicating the main application window. 

Note that the _NET_WM_WINDOW_TYPE_MENU should be set on torn-off managed windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagMenu = 1u << 3,
    ...,
};
```
