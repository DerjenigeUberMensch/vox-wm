# Graceful

Which means that a client will be sent a WM_DELETE_WINDOW window event (if available), and does not terminate the client if it fails to exit.

- [x] **Currently implemented.**

#### Value
```C
enum 
KillType
{
    ...,
    Graceful = 0,
    ...,
};
```
