# WTypeFlagDesktop

Indicates a desktop feature.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_DESKTOP` indicates a desktop feature.

This can include a single window containing desktop icons with the same dimensions as the screen, allowing the desktop environment to have full control of the desktop, without the need for proxying root window clicks.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagDesktop = 1u << 0,
    ...,
};
```
