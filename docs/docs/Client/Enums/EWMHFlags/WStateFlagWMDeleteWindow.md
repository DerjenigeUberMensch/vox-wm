# WStateFlagWMDeleteWindow

Indicates that a client has compatibility with the delete window.

This flag can be used to check wheter or not a client can be safely sent a delete message. (Window kill)

If not client should be killed directly or destroyed.

- [x] **Currently implemented.**

#### Atom
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
| WM_SAVE_YOURSELF  | Appendix C    | Save client state request. (deprecated)   |
| *WM_DELETE_WINDOW*| 4.2.8.1       | Request to delete top-level window.       |

It is expected that this table will grow over time.

Taken fron the ICCCM tronche: [https://tronche.com/gui/x/icccm/sec-4.html#WM_PROTOCOLS](https://tronche.com/gui/x/icccm/sec-4.html#WM_PROTOCOLS)

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WStateFlagWMDeleteWindow = 1u << 29,
    ...,
};
```
