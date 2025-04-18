# clientinitcolormap

## Signature
---

```C

void 
clientinitcolormap(
        Client *c,
        XCBGetWindowAttributes *wa
        );

```

---

## Arguments
| Arg           | Type                          | Description                                   |
| :---          | :---                          | :---                                          |
| *c*           | ***Client \****               | **A pointer to the client object.**    |
| *wa*           | ***XCBGetWindowAttributes \****               | **A pointer to the client object to intialize colormap with.**    |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: This function is a implementation detail, you might be looking for [manage()](../manage/).

## Usage

```C

Client *c = your_client_ptr;

XCBDisplay *display = _wm.dpy;
XCBWindow window = c->win;      /* client has the window attribut already intialized in this case */
XCBCookie cookie;
XCBGetWindowAttributes *wa;

cookie = XCBGetWindowAttributesCookie(display, window);
wa = XCBGetWindowAttributesReply(display, cookie);

clientinitcolormap(c, wa);

free(wa);

```
