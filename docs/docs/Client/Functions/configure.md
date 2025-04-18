# configure

## Signature

---

```C

void
configure(
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

The [*configure()*](../configure) function notifies windows of changes made from x,y,width,height and or border_width, and it also notifies them that they are currently being managed by the window manager as the ->override_redirect field is set to False.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

c->x = 10; /* Note you should never directly set the clients x field, instead use resize() to resize clients, but for this example we will */

configure(c);

```