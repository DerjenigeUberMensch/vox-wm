# setalwaysonbottom

## Signature

---

```C

void
setalwaysonbottom(
    Client *c, 
    bool state
    );

```

---

## Arguments

| Arg | Type | Description | 
| :--- | :--- | :--- |
| *c* | ***Client \**** | **A pointer to the client object** |
| *state* | ***bool*** | **The current bool state of the flag, true being on/active, false being off/disabled.** |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*setalwaysonbottom()*](../setalwaysonbottom) function sets the clients flag ***state*** for the enum WStateFlagBelow. This flag signifies to the window manager that clients should be always below others. Usually reserved for widget type windows, such as 'conky' and some background display windows.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) ***c***.  
:fontawesome-solid-circle-exclamation: Argument ***c*** must be **intialized** and **non [NULL](/C99_Data/Types/null)**.  

## Usage

```C

Client *c = your_client_ptr;
bool my_flag_state = false;

setalwaysonbottom(c, my_flag_state);

```