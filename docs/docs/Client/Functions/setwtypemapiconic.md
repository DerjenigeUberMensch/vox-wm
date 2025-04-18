# setwtypemapiconic

## Signature

---

```C

void
setwtypemapiconic(
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

The [*setwtypemapiconic()*](../setwtypemapiconic) function sets the WStateFlagMapIconic bit flag for field `ewmhflags` in the client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
bool is_wtype_mapping_iconic = true;

setwtypedesktop(c, is_wtype_mapping_iconic);

```