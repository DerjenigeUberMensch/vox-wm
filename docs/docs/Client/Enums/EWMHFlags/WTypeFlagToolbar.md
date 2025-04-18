# WTypeFlagToolbar

Indicates that a client is a tool-bar (torn off from the main application).

This should be used to detect whether a window should be grouped within an application,
and kept above other windows and treated as a bar-like window, see above.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_TOOLBAR` indicate toolbar windows, respectively (i.e. toolbars and menus "torn off" from the main application). 

Windows of this type may set the WM_TRANSIENT_FOR hint indicating the main application window. 

Note that the _NET_WM_WINDOW_TYPE_MENU should be set on torn-off managed windows

typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagToolbar = 1u << 2,
    ...,
};
```
