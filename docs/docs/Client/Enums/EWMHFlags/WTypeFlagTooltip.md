# WTypeFlagTooltip

Indicate that a client is a tooltip.

Which is simply those little helpful boxes that show up and tell you like what a thing does in a ui, these are typically found in qt(4/5) and some gtk applications.

These windows are usually override-redirect however, if not they should be treated as slightly above the parent windows type, but should not be above other windows.

There is no recommendation for this type of window, developer discretion is advised.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_TOOLTIP` indicates that the window in question is a tooltip, 

ie., a short piece of explanatory text that typically appear after the mouse cursor hovers over an object for a while. 

This property is typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagTooltip = 1u << 9,
    ...,
};
```
