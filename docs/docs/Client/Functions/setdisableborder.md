# setdisableborder

## Signature

---

```C

void
setdisableborder(
    Client *c, 
    bool is_border_disable
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *is_border_disable* | ***bool*** | **Whether or not to disable the current border** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setdisableborder()*](../setdisableborder) function sets the current visibility state of the client border if it is not 0, in which case this function would not visibly do anything.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
bool disable_border = true;

setdisableborder(c, disable_border);

```