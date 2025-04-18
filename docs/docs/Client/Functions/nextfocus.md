# nextfocus

## Signature

---

```C

Client *
nextfocus(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a Client * if there is a focus order client after this client, otherwise a NULL ptr is returned.

- [x] This function returns a Client *.

## Description

The [*nextfocus()*](../nextfocus) function gets the next available focus order client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *next_focused_client = your_client_ptr;

next_focused_client = nextfocus(c);

```