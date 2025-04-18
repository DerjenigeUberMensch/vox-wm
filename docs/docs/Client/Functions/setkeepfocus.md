# setkeepfocus

## Signature

---

```C

void
setkeepfocus(
    Client *c, 
    bool state
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *state* | ***bool*** | **The current bool state of the flag, true being on/active, false being off/disabled.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setkeepfocus()*](../setkeepfocus) function Sets the ClientFlagKeepFocus flag for the client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
bool is_wstate_keep_focus = true;

setkeepfocus(c, is_wstate_keep_focus);

```