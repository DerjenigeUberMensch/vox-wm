# maximizehorz

## Signature

---

```C

void
maximizehorz(
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

The [*maximizehorz()*](../maximizehorz) function maximizes the client to fit the screen conforming to strut boundaries of the bar and the boundaries of the screen in the x axis.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

maximizehorz(c);

```