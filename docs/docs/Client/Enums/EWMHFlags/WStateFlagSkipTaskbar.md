# WStateFlagSkipTaskbar

Indicates that a client should not be shown on the taskbar as a icon.

This is used only if there is a native bar on the wm.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_STATE_SKIP_TASKBAR` indicates that the window should not be included on a taskbar.

This hint should be requested by the application, i.e. it indicates that the window by nature is never in the taskbar. 

Applications should not set this hint if _NET_WM_WINDOW_TYPE already conveys the exact nature of the window.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagSkipTaskbar = 1u << 19,
    ...,
};
```
