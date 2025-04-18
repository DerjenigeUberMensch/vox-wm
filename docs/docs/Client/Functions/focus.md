# focus

## Signature

---

```C

void
focus(
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

The [*focus()*](../focus) function attempts to move focus to the specified client using [*focusrealize()*](../focusrealize) to determine the next client to focus, however by providing an argument, [*focus()*](../focus) attempts to use that client whenever possible.
## Usage

```C

Client *c = your_client_ptr;

focus(c); /* Try to focus my client */

focus(NULL); /* Try to focus the next available client */

```