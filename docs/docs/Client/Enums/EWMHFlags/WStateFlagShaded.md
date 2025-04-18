# WStateFlagShaded

Which indicates that a client is not fully visible.

In implementation(s) where there is a title bar this atom indicates,
that the client shouldnt be show, but the title bar should still be shown.

Think of it like having the title bar still there but the window contents are hidden.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_STATE_SHADED` indicates that the window is shaded.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagShaded = 1u << 18,
    ...,
};
```
