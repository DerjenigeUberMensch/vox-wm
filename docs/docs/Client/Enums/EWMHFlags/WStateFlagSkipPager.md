# WStateFlagSkipPager

Indicates that a client should not be shown on pagers as a icon.

This is used only if there is a native pager on the wm.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_STATE_SKIP_PAGER` indicates that the window should not be included on a Pager. 

This hint should be requested by the application, i.e. it indicates that the window by nature is never in the Pager. 

Applications should not set this hint if _NET_WM_WINDOW_TYPE already conveys the exact nature of the window.


#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagSkipPager = 1u << 20,
    ...,
};
```
