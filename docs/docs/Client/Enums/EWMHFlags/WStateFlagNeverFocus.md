# WStateFlagNeverFocus

Indicates that a client should not receive focus.  
This is highly recommened as usage otherwise can be undefined.  
(This breaks some stuff so follow it!)

- [x] **Currently implemented.**

#### Atom
`` 

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagNeverFocus = 1u << 30,
    ...,
};
```
