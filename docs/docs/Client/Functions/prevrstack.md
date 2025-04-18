# prevrstack

## Signature

---

```C

Client *
prevrstack(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a Client * if there is a restack order client before this client, otherwise a NULL ptr is returned.

- [x] This function returns a Client *.

## Description

The [*prevrstack()*](../prevrstack) function gets the previous available restack order client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *prev_restack_client = your_client_ptr;

prev_restack_client = prevrstack(c);

```