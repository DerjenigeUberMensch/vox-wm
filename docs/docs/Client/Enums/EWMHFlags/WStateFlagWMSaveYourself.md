# WStateFlagSaveYourself

Indicates that a client has compatibility with the save yourself flag.

This flag can be used to check whether or not a client can save itself (ie save its state).

This should be used when closing a client so the client has time to save its contents.

This allows you to make clients save themselves like saving a word document (Save As).

- [x] **Currently implemented.**

#### Type
`WM_PROTOCOL` 

To participate. Atoms can identify both standard protocols and private protocols specific to individual window managers.

All the protocols in which a client can volunteer to take part involve the window manager sending the client a ClientMessage event and the client taking appropriate action. For details of the contents of the event, see section 4.2.8. In each case, the protocol transactions are initiated by the window manager.

The *WM_PROTOCOLS* property is not required. 

If it is not present, the client does not want to participate in any window manager protocols.

The X Consortium will maintain a registry of protocols to avoid collisions in the name space. 

The following table lists the protocols that have been defined to date.

| Protocol          | Section       | Purpose                                   |
| :---              | :---          | :---                                      |
| WM_TAKE_FOCUS     | 4.1.7         | Assignment of input focus.                |
| *WM_SAVE_YOURSELF*| Appendix C    | Save client state request. (deprecated)   |
| WM_DELETE_WINDOW  | 4.2.8.1       | Request to delete top-level window.       |

It is expected that this table will grow over time.

Taken fron the ICCCM tronche: [https://tronche.com/gui/x/icccm/sec-4.html#WM_PROTOCOLS](https://tronche.com/gui/x/icccm/sec-4.html#WM_PROTOCOLS)

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagSaveYourself = 1u << 28,
    ...,
};
```
