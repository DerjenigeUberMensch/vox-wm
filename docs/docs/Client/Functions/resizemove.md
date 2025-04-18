# resizemove

## Signature

---

```C

void
resizemove(
    Client *c, 
    int16_t x, 
    int16_t y, 
    bool interact
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *x* | ***int16_t*** | **The new client x coordinate.** |
| *y* | ***int16_t*** | **The new client y coordinate.** |
| *interact* | ***bool*** | **Whether or not the client should be confined within the monitor during resize() operations.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*resizemove()*](../resizemove) function attempts to change a clients x/y coordinates. This change makes sure to be compliant with client's requested position restrictions imposed by the client. See [*applysizehints()*](../applysizehints) for information on client compliant requests. See [*resizeclient()*](../resizeclient) on storage and direct X11 changes for client's x, y, changes.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
int32_t x = 10;
int32_t y = 20;
bool confine_within_monitor = true

resizemove(c, x, y, confine_within_monitor);

```