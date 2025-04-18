# manage

## Signature

---

```C

Client *
manage(
    XCBWindow win, 
    void *replies
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *win* | ***XCBWindow*** | **The id to the window for the window manager to manage** |
| *replies* | ***void \**** | **The replies to use when managing the window.** |

## Return

Returns a Client * if the window specified and argument field 'replies' was managed successfully and is now in the management of the window manager, other wise a NULL ptr will be returned, in which likely one of the following. 

 1. The window is a 'root' window
2. The window is already managed.
3. The window is override redirect. 4.
 A memory allocation failed.

- [x] This function returns a Client *.

## Description

The [*manage()*](../manage) function attempts to manage the window provided if successfull it will treat the window as a Client for the foreseeable future until the window is unmapped/destroyed and or the window becomes override_redirect. Managing a window provides a user the ability to interact with the window, by use of keybinds and toggles, see toggle.h, further more it allows for better management of the window as it can more easily adhere to the window managers protocols.
## Usage

```C

Client *c;
XCBCookie request[ManageClientLAST];
void *replies[ManageClientLAST];
XCBWindow win = your_window_id;

managrequest(win, requests);
managereplies(requests, replies);

c = manage(win, replies);

```