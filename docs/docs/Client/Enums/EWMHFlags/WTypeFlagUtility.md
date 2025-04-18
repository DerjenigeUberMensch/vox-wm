# WTypeFlagUtility

Which indicates that a client should be kept above regular floating/non-floating windows.

However should not be kept above other more important windows (stacking wise),
such as dialog boxes, modal dialog boxes, toolbars, splash, notifications, etc...

This flag should be used for stacking in a manner slightly more important than regular floating windows.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_UTILITY` indicates a small persistent utility window, such as a palette or toolbox. 

It is distinct from type TOOLBAR because it does not correspond to a toolbar torn off from the main application. 

It's distinct from type DIALOG because it isn't a transient dialog, the user will probably keep it open while they're working. Windows of this type may set the WM_TRANSIENT_FOR hint indicating the main application window.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagUtility = 1u << 4,
    ...,
};
```
