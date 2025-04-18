# managerequest

## Signature

---

```C

void
managerequest(
    XCBWindow win, 
    XCBCookie *requests
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *win* | ***XCBWindow*** | **The id to the window to request info from for manage()** |
| *requests* | ***XCBCookie \**** | **The requests to fill back.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*managerequest()*](../managerequest) function ***requests*** the XServer for properties relating to the intial management of a window, and fill such data in the correponding field ***'requests'*** 
## Usage

```C

XCBCookie requests[ManageClientLAST];

XCBWindow win = your_window_id;

managerequest(win, requests);

```