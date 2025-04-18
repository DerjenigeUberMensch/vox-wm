# setclientwtype

## Signature

---

```C

void
setclientwtype(
    Client *c, 
    XCBAtom atom, 
    bool state
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *atom* | ***XCBAtom*** | **The atom to add/remove WType (Window Type) from.** |
| *state* | ***bool*** | **Whether or not to add/remove an atom, true being to add the atom specified, false being to remove it.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setclientwtype()*](../setclientwtype) function notifies the Prop queue, via the PropListenArg function, to update the specified argument ***'c'*** window type ***atom*** and its current provided state.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
XCBAtom atom = netatom[NetWMWindowTypeDialog];
bool add_atom = true;

setclientwtype(c, atom, add_atom);

```