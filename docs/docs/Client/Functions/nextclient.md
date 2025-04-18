# nextclient

## Signature

---

```C

Client *
nextclient(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a Client * if there is a mapped order client after this client, otherwise a NULL ptr is returned.

- [x] This function returns a Client *.

## Description

The [*nextclient()*](../nextclient) function gets the next available mapping order client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *next_client = your_client_ptr;

next_mapped_client = nextclient(c);

```