# setclientdesktop

## Signature

---

```C

void
setclientdesktop(
    Client *c, 
    Desktop *desktop
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *desktop* | ***Desktop \**** | **A pointer to the desktop object** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setclientdesktop()*](../setclientdesktop) function The [*setclientdesktop()*](../setclientdesktop) functions changes the current clients correponding desktop, and or adds it if not already set. However does not visually restack only locally is this change set.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***, and ***desktop***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  
:fontawesome-solid-circle-exclamation: Argument ***desktop*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
Desktop *desk = new_desktop_ptr;

/* Note: Client is only internally updated */
setclientdesktop(c, desk);
/* Update externally the changes we made (optional).
 Note for this example it only uses showhide() however in general it would be recommended to use arrange() on both desktops as to maintain stack order. */
showhide(c);

```