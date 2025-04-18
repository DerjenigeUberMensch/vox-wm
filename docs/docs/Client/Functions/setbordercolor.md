# setbordercolor

## Signature

---

```C

void
setbordercolor(
    Client *c, 
    uint8_t red, 
    uint8_t green, 
    uint8_t blue
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *red* | ***uint8_t*** | **The red (ARGB) color level 0-255 of the Client border.** |
| *green* | ***uint8_t*** | **The green (ARGB) color level 0-255 of the Client border.** |
| *blue* | ***uint8_t*** | **The blue (ARGB) color level 0-255 of the Client border.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setbordercolor()*](../setbordercolor) function sets the clients border color using the provided RGB values.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
uint8_t red = 100;
uint8_t green = 50;
uint8_t blue = 255;

setbordercolor(c, red, green, blue);

```