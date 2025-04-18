# setmapstate

## Signature

---

```C

void
setmapstate(
    Client *c, 
    enum WMMapState state
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *state* | ***enum WMMapState*** | **The WMMapState to set for client.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setmapstate()*](../setmapstate) function sets the WMMapState for a client, that being mapped and or unmmaped.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
enum WMMapState newState = WMMapStateMapped;

setmapstate(c, newState);

```