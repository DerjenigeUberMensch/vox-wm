# resize

## Signature

---

```C

void
resize(
    Client *c, 
    int32_t x, 
    int32_t y, 
    int32_t width, 
    int32_t height, 
    bool interact
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *x* | ***int32_t*** | **The new client x coordinate.** |
| *y* | ***int32_t*** | **The new client y coordinate.** |
| *width* | ***int32_t*** | **The new client width.** |
| *height* | ***int32_t*** | **The new client height.** |
| *interact* | ***bool*** | **Whether or not the client should be confined within the monitor during resize() operations.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*resize()*](../resize) function attempts to change a clients x/y coordinates and or width/height size. This change makes sure to be compliant with client's requested min/max size and or certain resizing restrictions imposed by the client. See [*applysizehints()*](../applysizehints) for information on client compliant requests. See [*resizeclient()*](../resizeclient) on storage and direct X11 changes for client's x, y, width, and or ***height*** changes.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
int32_t x = 10;
int32_t y = 20;
int32_t width = 500;
int32_t height = 500;
bool confine_within_monitor = true

resize(c, x, y, w, h, confine_within_monitor);

```