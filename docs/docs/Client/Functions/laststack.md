# laststack

## Signature

---

```C

Client *
laststack(
    Desktop *desktop
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *desktop* | ***Desktop \**** | **A pointer to the desktop object** |

## Return

Returns the last 'stack' client in the desktop specified if applicable, otherwise a NULL ptr is returned

- [x] This function returns a Client *.

## Description

The [*laststack()*](../laststack) function gets the last 'stack' client in ***desktop*** if any are available.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***desktop***.  
:fontawesome-solid-circle-exclamation: Argument ***desktop*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Desktop *desk = your_desktop_ptr;
Client *last_stack_client ;

last_stack_client = laststack(desk);

```