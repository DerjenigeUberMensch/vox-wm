# setborderalpha

## Signature

---

```C

void
setborderalpha(
    Client *c, 
    uint8_t alpha
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *alpha* | ***uint8_t*** | **The opacity level 0-255 of the Client border.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setborderalpha()*](../setborderalpha) function sets the clients border opacity, i.e ***alpha*** from a range of 0-255.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
int8_t no_opacity = UINT8_MAX;

setborderalpha(c, no_opacity);

```