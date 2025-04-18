# WStateFlagFullscreen

Indicates that a client is in fullscreen mode.
This can be set by either the window manager or application.
It is required by spec that the application should remove decorations, and should fill up the screen.


It is recommened that the client be maximized and decoration(s) be hidden.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_STATE_FULLSCREEN` indicates that the window should fill the entire screen and have no window decorations. 


Additionally the Window Manager is responsible for restoring the original geometry after a switch from fullscreen back to normal window. 

For example, a presentation program would use this hint. 

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagDemandAttention = 1u << 22,
    ...,
};
```
