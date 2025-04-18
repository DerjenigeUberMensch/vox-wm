# nextstack

## Signature

---

```C

Client *
nextstack(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a Client * if there is a stack order client after this client, otherwise a NULL ptr is returned.

- [x] This function returns a Client *.

## Description

The [*nextstack()*](../nextstack) function gets the next available stack order client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *next_stack_client = your_client_ptr;

next_stack_client = nextstack(c);

```