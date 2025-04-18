# WTypeFlagDock

Indicates that a client should be kept above others.

As it is a bar/pannel (such as polybar/conky) like feature (bar as in desktop taskbar).

Do note that due to some focusing issues with the way the window manager is run, these windows should never be focused directly see SHOULDNEVERFOCUS().

- [x] **Currently implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_DOCK` indicates a dock or panel feature.

Typically a Window Manager would keep such windows on top of all other windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagDock = 1u << 1,
    ...,
};
```
