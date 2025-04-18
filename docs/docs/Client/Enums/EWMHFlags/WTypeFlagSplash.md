# WTypeFlagSplash


Indicates that a client is a splash window, which might be a logo window, startup window, etc...

This should be kept above almost every other window, aside from dock like windows.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_SPLASH` indicates that the window is a splash screen displayed as an application is starting up.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagSplash = 1u << 5,
    ...,
};
```
