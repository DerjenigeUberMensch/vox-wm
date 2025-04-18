# PropLAST

Last identifier, used for making array sizes.

Thought this should not be used aside from internal __PropHandler__ and related functions.

- [x] **Currently implemented.**


### Example
```C

/* Note internal structure DONOT USE! */
__PropHandler__ __prophandler__[PropLAST];
```

#### Value

```C
enum 
PropertyType
{
    ...,
    PropLAST = 22,
    ...,
};
```
