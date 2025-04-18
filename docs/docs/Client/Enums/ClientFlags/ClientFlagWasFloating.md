# ClientFlagWasFloating

This flag indicates that a client is was previously floating.

- [x] **Currently implemented.**

#### Value
```C
enum 
ClientFlags
{
    ...,
    ClientFlagWasFloating = 1u << 1,
    ...,
};
```