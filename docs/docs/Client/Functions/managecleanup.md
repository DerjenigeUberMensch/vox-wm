# managecleanup

## Signature

---

```C

void
managecleanup(
    void **replies
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *replies* | ***void \*\**** | **The replybacks from managereplies() to free** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*managecleanup()*](../managecleanup) function frees any and all resulting data from the replybacks generated from [*managereplies()*](../managereplies) as such should ***always*** be used ***after*** calling [*managereplies()*](../managereplies).

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***replies***.  
:fontawesome-solid-circle-exclamation: Replies are freed after this function returns.  
:fontawesome-solid-circle-exclamation: Argument ***replies*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

XCBWindow win = your_window_id;
XCBCookie requests[ManageClientLAST];
void *replies[ManageClientLAST];

managerequest(win, requests);
managereplies(requests, replies);

managecleanup(replies);

```