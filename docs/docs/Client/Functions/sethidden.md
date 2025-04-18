# sethidden

## Signature

---

```C

void
sethidden(
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

The [*sethidden()*](../sethidden) function Sets the WStateFlagHidden flag for the client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
bool is_wstate_hidden = true;

sethidden(c, is_wstate_hidden);

```