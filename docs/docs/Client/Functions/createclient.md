# createclient

## Signature

---

```C

Client *
createclient(
    void
    );

```

---

## Arguments

There are no arguments in this function.

## Return

Returns a Client * if memory for the client was successfully allocated, a NULL pointer otherwise.

- [x] This function returns a Client *.

## Description

The [*createclient()*](../createclient) function returns a empty block of memory that has all data intialized and ready to be used, this include field member c->decor being a allocated chunk of memory.
## Usage

```C

Client *c = NULL;

c = createclient();

```