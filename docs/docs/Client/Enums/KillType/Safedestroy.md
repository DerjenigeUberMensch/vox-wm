# Safedestroy

Which means that a client will be sent a WM_DELETE_WINDOW window event (if available),
and if the client fails to exit a predefined condition (timer usually) will be set,
afterwards which if a client has not exited a KillClient request will be sent to the client,
and again if the client fails to exit a predefined condition a DestroyWindow request will be sent to the client.
In which point will destroy the window, this may fail sometimes though with higher privilege windows.

Currently partially implemented, defaults to destroying window as per partial implementation.


- [x] **Partially implemented.**

#### Value
```C
enum 
KillType
{
    ...,
    Safedestroy = 1,
    ...,
};
```
