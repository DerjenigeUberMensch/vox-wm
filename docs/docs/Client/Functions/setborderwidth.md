# setborderwidth

## Signature

---

```C

void
setborderwidth(
    Client *c, 
    uint16_t border_width
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *border_width* | ***uint16_t*** | **The border width size.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setborderwidth()*](../setborderwidth) function sets the clients border width. See enum ClientFlagDisableBorder for usage if disabled

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
uint16_t border_width = 15;

setborderwidth(c, border_width);

```