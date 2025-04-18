# managereplies

## Signature

---

```C

void
managereplies(
    XCBCookie *requests, 
    void *replies
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *requests* | ***XCBCookie \**** | **The requests for managing the client, from managerequest()** |
| *replies* | ***void \**** | **The replies to fillback unto callee.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*managereplies()*](../managereplies) function ***requests*** for the replyback from the XServer for properties relating to the intial management of a window, and fills such data in the corresponding field, ***'replies'***
## Usage

```C

XCBCookie request[ManageClientLAST];
void *replies[ManageClientLAST];
XCBWindow win = your_window_id;

managrequest(win, requests);

managereplies(requests, replies);

```