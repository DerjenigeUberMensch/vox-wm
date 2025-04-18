# clientinittrans

## Signature

---

```C

void
clientinittrans(
    Client *c, 
    XCBWindow trans
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *trans* | ***XCBWindow*** | **The transient window to intialize client with** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*clientinittrans()*](../clientinittrans) function intializes the clients transient state.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

XCBDisplay *display = _wm.dpy;
XCBWindow win = c->win;
XCBCookie trans_cookie = XCBGetTransientForHintCookie(display, win);

XCBWindow trans;
uint8_t trans_status = XCBGetTransientForHintReply(display, cookie, &trans);
/* If no trans make sure intialized as 0 */
if (!trans_status)
{
    trans = 0;
}

clientinittrans(c, trans);

```