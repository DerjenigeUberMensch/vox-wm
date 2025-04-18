# WStateFlagAbove

Indicates that a client window should remain above most other windows.  
This property is also known as **AlwaysOnTop**.  

It is strongly recommended to keep these windows above all others,  
except for specific types like docks, bars, or splash screens.  

- [x] **Currently implemented.**



#### Atom
`_NET_WM_STATE_ABOVE` indicates that the window should be on top of most windows.


#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagAbove = 1u << 23,
    ...,
};
```
