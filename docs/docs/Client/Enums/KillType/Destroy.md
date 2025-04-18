# Destroy

A DestroyWindow request will be sent to the client, and usually kill the client, unless the client has some form of privilege escalation compared to the window manager.

Some reports of VirtualBox not exiting are cited.

- [x] **Currently implemented.**

#### Value
```C
enum 
KillType
{
    ...,
    Destroy = 2,
    ...,
};
```
