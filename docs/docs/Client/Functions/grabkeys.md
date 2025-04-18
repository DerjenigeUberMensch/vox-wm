# grabkeys

## Signature

---

```C

void
grabkeys(
    void
    );

```

---

## Arguments

There are no arguments in this function.

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*grabkeys()*](../grabkeys) function is a archaic remnant of the [dwm](https://dwm.suckless.org/) source translation to xcb. But to be brief this function grabs all the keys needed to perform user defined callbacks. Furthermore it should be called only udring mappingnotifies (key mapping) and at the start of the WM, as such should under no circumstance continue operation if function has failed unless user has specified another form of window manager termination.
## Usage

```C

grabkeys();

```