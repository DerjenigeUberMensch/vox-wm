# clientinitwtype

## Signature

---

```C

void
clientinitwtype(
    Client *c, 
    XCBWindowProperty *wtypeprop
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *wtypeprop* | ***XCBWindowProperty \**** | **A pointer to the window type object.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*clientinitwtype()*](../clientinitwtype) function intializes the clients geometry.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

XCBDisplay *display = _wm.dpy;
XCBWindow window = c->win;      /* client has the window attribut already intialized in this case */
XCBCookie cookie;
XCBGetWindowProperty *wp;

const uint8_t NO_BYTE_OFFSET = 0;
const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;

cookie = XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, false, XCB_ATOM_ATOM);
wp = XCBGetWindowPropertyReply(display, cookie);

clientinitwtype(c, wp);

free(wp);

```