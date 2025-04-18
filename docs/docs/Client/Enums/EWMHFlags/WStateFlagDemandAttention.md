# WStateFlagDemandAttention

Indicates that a client is urgent and should be brought to attention somehow.  


It is recommened to focus and bring client up the stack, but developer discretion is adviced.  

- [x] **Currently implemented.**

#### Atom
`_NET_WM_STATE_DEMANDS_ATTENTION` indicates that some action in or with the window happened.  


For example, it may be set by the Window Manager if the window requested activation but the Window Manager refused it, or the application may set it if it finished some work.   
This state may be set by both the Client and the Window Manager. 
It should be unset by the Window Manager when it decides the window got the required attention (usually, that it got activated).  

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagDemandAttention = 1u << 25,
    ...,
};
```
