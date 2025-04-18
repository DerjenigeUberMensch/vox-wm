# applysizehints

## Signature
---

```C

bool
applysizehints(
        Client *c,
        int32_t *x,
        int32_t *y,
        int32_t *width,
        int32_t *height
        );

```

---

## Arguments
| Arg           | Type                          | Description                                   |
| :---          | :---                          | :---                                          |
| *c*           | ***Client \****               | **A pointer to the client containing the sizehints.**    |
| *x*           | ***int32_t \****              | **The X coordinate pointer to be modified.**  |
| *y*           | ***int32_t \****              | **The Y coordinate pointer to be modified.**  |
| *width*       | ***int32_t \****              | **The Width size pointer to be modified.**    |
| *height*      | ***int32_t \****              | **The Height size pointer to be modified.**   |

## Return

If the return is true then a [resize()](#resize) should be applied to the client pointer ***c*** otherwise no action is required if false.

- [x] This function returns a boolean value.


## Description

The [*applygravity()*](#applygravity) function applies the bit ***gravity*** to the ***x*** and ***y*** coordinate(s) provided.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the arguments ***x***, ***y***, ***width***, ***height***.   
:fontawesome-solid-circle-exclamation: Argument ***x*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: Argument ***y*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: Argument ***width*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: Argument ***height*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: This function is an *implementation detail.* You might be looking for [resize()](#resize).

## Usage
```C
Client *c = your_client_ptr;    /* Client should be intialized before usage */

int32_t new_x = 100;
int32_t new_y = 100;
int32_t new_width = 50;
int32_t new_height = 50
bool needs_resize = applysizehints(c, &new_x, &new_y, &new_width, &new_height);
```
