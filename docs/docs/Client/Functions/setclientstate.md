# setclientstate

## Signature

---

```C

void
setclientstate(
    Client *c, 
    uint8_t state
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *state* | ***uint8_t*** | **The WMState to set the specified client.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setclientstate()*](../setclientstate) function tells the XServer and the correponding client to change its wmatom[WMState] value to the one provided in argument ***'state'***

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
uint8_t state = XCB_WINDOW_NORMAL_STATE;

setclientstate(c, state);

```