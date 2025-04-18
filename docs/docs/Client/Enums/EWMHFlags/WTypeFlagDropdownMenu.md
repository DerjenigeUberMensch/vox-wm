# WTypeFlagDropdownMenu

Indicates that a client is a drop down menu.

These windows are usually override-redirect however, if not should be treated as regular floating windows, and may be grouped, though not required with the parent application.

This is recommened though.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_DROPDOWN_MENU` indicates that the window in question is a dropdown menu,

i.e., the kind of menu that typically appears when the user clicks on a menubar, as opposed to a popup menu which typically appears when the user right-clicks on an object. 

This property is typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagDropdownMenu = 1u << 7,
    ...,
};
```
