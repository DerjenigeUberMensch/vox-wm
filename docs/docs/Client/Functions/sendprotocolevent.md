# sendprotocolevent

## Signature

---

```C

void
sendprotocolevent(
    Client *c, 
    XCBAtom protocol
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *protocol* | ***XCBAtom*** | **The WMProtocol event to send.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*sendprotocolevent()*](../sendprotocolevent) function sends a ClientMessage to the specified client, with the WMProtocol being sent as the type and argument ***'protocol'*** being the sent atom type.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
XCBAtom takefocus = wmatom[WMTakeFocus];
XCBAtom saveyourself = wmatom[WMSaveYourself];
XCBAtom deletewindow = wmatom[WMDeleteWindow];

/* Tells client that they are currently being focused */
sendprotocolevent(c, takefocus);
/* Tells client to save all its settings (deprecated) */
sendprotocolevent(c, saveyourself);
/* Tells a client to 'kill/terminate' itself, i.e delete window*/
sendprotocolevent(c, deletewindow);

```