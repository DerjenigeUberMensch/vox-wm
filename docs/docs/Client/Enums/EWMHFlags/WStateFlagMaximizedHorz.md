# WStateFlagMaximizedHorz

!!! Warning
    This is a **[VLT](/Docs/#vlt)** feature.

Which indicates that a client is maximize horizontally.
This atom is set by applications requesting maximization/minimization by the horizontal axis.

This atom is ***NOT*** recommended to be read as its data is not fixed.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_STATE_MAXIMIZED_HORZ` indicates that the window is horizontally maximized. 

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagMaximizedHorz = 1u << 17,
    ...,
};
```
