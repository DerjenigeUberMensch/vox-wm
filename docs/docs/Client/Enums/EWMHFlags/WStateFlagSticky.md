# WStateFlagSticky

Indicates that a client should be "sticky", and not be hidden on desktop change.

These windows are usually not directly set as sticky however may be set later on by the application.

These windows also arent usually set by application if at all, and are more of a window manager feature.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_STATE_STICKY` indicates that the Window Manager SHOULD keep the window's position fixed on the screen, even when the virtual desktop scrolls.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagSticky = 1u << 15,
    ...,
};
```
