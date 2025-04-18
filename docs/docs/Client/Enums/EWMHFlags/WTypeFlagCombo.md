# WTypeFlagCombo

Which indicates that a client is tooltip-like ~es.

These windows are usually override-redirect however,

if not windows should be roughly handled like tooltips,
and should have slightly higher than tooltip windows, but again should be not conflicting with any windows.

There is no recommendation for this type of window, developer discretion is advised.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_COMBO` should be used on the windows that are popped up by combo boxes. 

An example is a window that appears below a text field with a list of suggested completions. 

This property is typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagCombo = 1u << 11,
    ...,
};
```
