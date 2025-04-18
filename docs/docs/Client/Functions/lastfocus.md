# lastfocus

## Signature

---

```C

Client *
lastfocus(
    Desktop *desktop
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *desktop* | ***Desktop \**** | **A pointer to the desktop object** |

## Return

Returns the last focused client in the desktop specified if applicable, otherwise a NULL ptr is returned

- [x] This function returns a Client *.

## Description

The [*lastfocus()*](../lastfocus) function gets the last focused client in ***desktop*** if any are available.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***desktop***.  
:fontawesome-solid-circle-exclamation: Argument ***desktop*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Desktop *desk = your_desktop_ptr;
Client *last_focused_client ;

last_focused_client = lastfocus(desk);

```