# ManageClientLAST

Last identifier, used for making array sizes.

- [x] **Currently implemented.**

### Example
```C
XCBCookie cookies[ManageClientLAST];
void *replies[ManageClientLAST];
```

#### Value
```C
enum 
ManageClientProperties
{
    ...,
    ManageClientLAST = 16,
    ...,
};
```