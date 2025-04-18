# WTypeFlagDnd

Indicates that a client is currently being dragged by the cursor.

These windows are usually override-redirect however, developer discretion is advised for implementation.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_DND` indicates that the window is being dragged.

Clients should set this hint when the window in question contains a representation of an object being dragged from one place to another. 

An example would be a window containing an icon that is being dragged from one file manager window to another. 

This property is typically used on override-redirect windows.


#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagDnd = 1u << 12,
    ...,
};
```
