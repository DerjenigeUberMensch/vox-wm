# grabbuttons

## Signature

---

```C

void
grabbuttons(
    Client *c, 
    bool focused
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *focused* | ***bool*** | **Whether or not a client should be or is considered 'focused'** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*grabbuttons()*](../grabbuttons) function is a archaic remnant of the [dwm](https://dwm.suckless.org/) source translation to xcb. But to be brief this function grabs all the buttons needed to perform user defined callbacks, and to provide for clicking other windows, yes we need this in order to replay the pointer back to the window so external clicks register.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

grabbuttons(c, false);

```