# WStateFlagBelow

Indicates that a client should be below most other clients.  
This property is also known as **AlwaysBelow** or **KeepBelow**.  

It is strongly recommended to keep these windows below all others,  

- [x] **Currently implemented.**



#### Atom
`_NET_WM_STATE_BELOW` indicates that the window should be below most windows



#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagAbove = 1u << 24,
    ...,
};
```
