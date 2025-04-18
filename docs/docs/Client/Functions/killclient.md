# killclient

## Signature

---

```C

void
killclient(
    Client *c, 
    enum KillType kill_type
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *kill_type* | ***enum KillType*** | **The kill operation to perform on the specified client** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*killclient()*](../killclient) function sends a signal to the specified client to kill further client operation.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;

/* Kill the client gracefuly */
killclient(c, Graceful);
/* Kill the client using a safe destroy method */
killclient(c, Safedestroy);
/* Kill the client using a hard destroy method */
killclient(c, Destroy);
/* ... (Left for future implementations) */

```