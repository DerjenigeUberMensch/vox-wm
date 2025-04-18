# resizeclient

## Signature

---

```C

void
resizeclient(
    Client *c, 
    int16_t x, 
    int16_t y, 
    int16_t width, 
    int16_t height
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *x* | ***int16_t*** | **The new client x coordinate.** |
| *y* | ***int16_t*** | **The new client y coordinate.** |
| *width* | ***int16_t*** | **The new client width.** |
| *height* | ***int16_t*** | **The new client height.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*resizeclient()*](../resizeclient) function changes the dimentions of a client via the specified x, y, ***width*** and or height. This function should be noted as not having any security checks and may have hugely offset ***x*** and or ***y*** coordinates set, ***width*** possible negative window width/height, (this breaks windows do not do!). Furthermore this function has a special method of resizing as it does not 'apply' changes if the client is not visible, see [*ISVISIBLE()*](../ISVISIBLE) for details.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
int16_t x = 0;
int16_t y = 0;
int16_t width = 50;
int16_t height = 50;

resizeclient(c, x, y, w, h);

```