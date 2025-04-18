# WTypeFlagPopupMenu

Indicates that a client is a popup dialog box, or popup menu.

These windows are usually override-redirect however, 
if not should be treated as temporarily more important than modal dialog boxes, and dialog boxes.

After some predetermined amount of time should be reverted to and treated as regular dialog boxes.

Though this behaviour is not required.

It is still highly highly recommened to treat these windows as regular dialog boxes.

- [x] **Partially implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_POPUP_MENU` indicates that the window in question is a popup menu

ie., the kind of menu that typically appears when the user right clicks on an object, as opposed to a dropdown menu which typically appears when the user clicks on a menubar. 

This property is typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagPopupMenu = 1u << 8,
    ...,
};
```
