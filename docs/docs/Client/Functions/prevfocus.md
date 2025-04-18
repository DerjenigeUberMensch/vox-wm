# prevfocus

## Signature

---

```C

Client *
prevfocus(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a Client * if there is a focus order client before this client, otherwise a NULL ptr is returned.

- [x] This function returns a Client *.

## Description

The [*prevfocus()*](../prevfocus) function gets the previous available focus order client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *prev_focused_client = your_client_ptr;

prev_focused_client = prevfocus(c);

```