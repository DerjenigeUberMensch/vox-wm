# WTypeFlagNormal

Indicates that a client is a normal top level window.

It is a suggestion that these windows dont have any special properties set to them.

There is no recommendation for this type of window, developer discretion is advised.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_NORMAL`  indicates that this is a normal, top-level window, either managed or override-redirect

Managed windows with neither _NET_WM_WINDOW_TYPE nor WM_TRANSIENT_FOR set MUST be taken as this type. 

Override-redirect windows without _NET_WM_WINDOW_TYPE, must be taken as this type, whether or not they have WM_TRANSIENT_FOR set.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagNormal = 1u << 13,
    ...,
};
```
