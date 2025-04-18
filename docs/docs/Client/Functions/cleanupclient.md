# cleanupclient

## Signature
---

```C

void 
cleanupclient(
        Client *c
        );

```

---

## Arguments
| Arg           | Type                          | Description                                   |
| :---          | :---                          | :---                                          |
| *c*           | ***Client \****               | **A pointer to the client to which data is freed.**    |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description
The [cleanupclient()](#cleanupclient) function frees internal data of the passed client object ***c***, and frees the given object ***c***.

:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: Argument ***c*** is freed after this function exists.
:fontawesome-solid-circle-exclamation: This function is an *implementation detail.* you might be looking for [unmanage()](../unmanage/). 

## Usage
```C
Client *c = your_client_ptr;   /* Client should be intialized before usage */

cleanupclient(c);

c = NULL;   /* c now holds a dangling pointer so set to NULL */

```
