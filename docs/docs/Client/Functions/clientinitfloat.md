# clientinitfloat

## Signature

---

```C

void
clientinitfloat(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*clientinitfloat()*](../clientinitfloat) function intializes and detects whether or not a client is detected, to be a [floating](/Docs/#floating) type window.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

clientinitfloat(c);

```