# setclientpid

## Signature

---

```C

void
setclientpid(
    Client *c, 
    pid_t pid
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *pid* | ***pid_t*** | **The Proccess IDentification number to set for client.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setclientpid()*](../setclientpid) function sets Process IDentification (PID) number for the specified client.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
pid_t pid = MY_PID;

setclientpid(c, pid);

```