# setbordercolor32

## Signature

---

```C

void
setbordercolor32(
    Client *c, 
    uint32_t argb_color
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *argb_color* | ***uint32_t*** | **The ARGB color shift to be used.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setbordercolor32()*](../setbordercolor32) function sets the clients border color using the provided ARGB value shift, and is formated as follows:  
BLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24)

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
uint8_t red = 100;
uint8_t green = 50;
uint8_t blue = 255;
uint8_t alpha = 255;
uint32_t bordercol32 = blue + (green << 8) + (red << 16) + (alpha << 24);

setbordercolor32(c, bordercol32);

```