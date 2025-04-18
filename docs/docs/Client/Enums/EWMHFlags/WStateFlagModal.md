# WStateFlagModal

Indicates that a client is a modal dialog box.

A modal dialog box is basically a more "important" dialog box.

It is recommened to have this to be atleast a dialog box, but should have its own type as modal.

Usually this atom has a pair with _NET_WM_WINDOW_TYPE_DIALOG, however is not guaranteed.

- [x] **Currently implemented.**

#### Atom
`_NET_WM_STATE_MODAL` indicates that this is a modal dialog box.

If the WM_TRANSIENT_FOR hint is set to another toplevel window, the dialog is modal for that window; if WM_TRANSIENT_FOR is not set or set to the root window the dialog is modal for its window group.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagModal = 1u << 14,
    ...,
};
```
