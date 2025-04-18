# clientinitgeom

## Signature

---

```C

void
clientinitgeom(
    Client *c, 
    XCBWindowGeometry *geometry
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *geometry* | ***XCBWindowGeometry \**** | **A pointer to the window geometry object.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*clientinitgeom()*](../clientinitgeom) function intializes the clients geometry.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

XCBDisplay *display = _wm.dpy;
XCBWindow window = c->win;    /* client has the window attribute intialized in this case */
XCBCookie cookie;
XCBWindowGeometry *wg;

cookie = XCBGetWindowGeometryCookie(display, window);
wg = XCBGetWindowGeometryReply(display, cookie);

clientinitgeom(c, wg);

free(wg);

```