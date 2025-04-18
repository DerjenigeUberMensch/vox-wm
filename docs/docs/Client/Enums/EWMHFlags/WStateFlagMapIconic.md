# WStateFlagMapIconic

Indicates whether a client is normal, or iconic.

NOTE: [vox-wm](https://github.com/DerjenigeUberMensch/vox-wm) does not currently support *withdrawn* windows.


This flag can be used to check whether a client is iconic, or normal,
this is due to the fact that widthdrawn windows arent managed (currently).


- [x] **Currently implemented.**

#### State
`WM_HINTS.intial_state`  The state field describes the window manager's idea of the state the window is in, which may not match the client's idea as expressed in the initial_state field of the WM_HINTS property (for example, if the user has asked the window manager to iconify the window).


If it is **WithdrawnState** the window manager will either change the state field's value to WithdrawnState or it will remove the WM_STATE property entirely.  

If it is **NormalState**, the window manager believes the client should be animating its window.  

If it is **IconicState**, the client should animate its icon window. In either state, clients should be prepared to handle exposure events from either window.  


Adding other fields to this property is reserved to the X Consortium. Values for the state field other than those defined in the above table are reserved for use by X Consortium.


The following table lists the WM_STATE.state values:

| State             | Value |
| :---              | :---  |
| WithdrawnState    | 0     |
| NormalState       | 1     |
| IconicState       | 3     |

Taken fron the ICCCM tronche: [https://tronche.com/gui/x/icccm/sec-4.html](https://tronche.com/gui/x/icccm/sec-4.html)

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagMapIconic = 1u << 31,
    ...,
};
```
