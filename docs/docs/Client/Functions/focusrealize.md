# focusrealize

## Signature

---

```C

Client *
focusrealize(
    Client *c
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |

## Return

Returns a client pointer to the client that was decided to be considered 'focused' under our managmenet, otherwise NULL if no client was focused.

- [x] This function returns a Client *.

## Description

The [*focusrealize()*](../focusrealize) function attemps to find the next available client to focus within our builtin system of focusing clients, and like [*focus()*](../focus) attempts to use the provided arg ***c*** when ever possible. This however does not focus any client directly and must be focused using [*focus()*](../focus) in order to update the windows state in the XServer. Basically this just updates our internal focus order, but doesnt update the XServer focus order. This can be useful when performing certain stacking optimizations.
## Usage

```C

Client *c = your_client_ptr;

c = focusrealize(c);

/* perform optimizations here */

focus(c);

```