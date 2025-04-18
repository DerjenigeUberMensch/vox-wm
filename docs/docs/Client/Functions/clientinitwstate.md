# clientinitwstate

## Signature

---

```C

void
clientinitwstate(
    Client *c, 
    XCBWindowProperty *wstateprop
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *wstateprop* | ***XCBWindowProperty \**** | **A pointer to the window state object.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*clientinitwstate()*](../clientinitwstate) function intializes the clients window state(s).

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

cookie = XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, false, XCB_ATOM_ATOM);
wp = XCBGetWindowPropertyReply(display, cookie);

clientinitwstate(c, wp);

free(wp);

```