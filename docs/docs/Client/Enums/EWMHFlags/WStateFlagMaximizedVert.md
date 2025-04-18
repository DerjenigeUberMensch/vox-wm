# WStateFlagMaximizedVert

!!! Warning
    This is a **[VLT](/Docs/#vlt)** feature.

Which indicates that a client is maximize vertically.
This atom is set by applications requesting maximization/minimization by the vertical axis.

This atom is ***NOT*** recommended to be read as its data is not fixed.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_STATE_MAXIMIZED_VERT` indicates that the window is vertically maximized. 

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagMaximizedVert = 1u << 16,
    ...,
};
```
