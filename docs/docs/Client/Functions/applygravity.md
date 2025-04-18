#  applygravity

## Signature
---

```C

void 
applygravity(
        enum XCBBitGravity gravity, 
        int32_t *x, 
        int32_t *y, 
        uint32_t width, 
        uint32_t height, 
        uint32_t border_width
        );

```

---

## Arguments
| Arg           | Type                          | Description                                   |
| :---          | :---                          | :---                                          |
| *gravity*     | ***enum XCBBitGravity***      | **The Bit gravity related to the Window.**    |
| *x*           | ***int32_t \****              | **The X coordinate pointer to be modified.**  |
| *y*           | ***int32_t \****              | **The Y coordinate pointer to be modified.**  |
| *width*       | ***uint32_t***                | **The Width size of the window.**             |
| *height*      | ***uint32_t***                | **The Height size of the window.**            |
| *border_width*| ***uint32_t***                | **The Border Width of the window.**           |

## Return

:octicons-x-circle-fill-12: This function does not return a value.

## Description

The [*applygravity()*](#applygravity) function applies the bit ***gravity*** to the ***x*** and ***y*** coordinate(s) provided.

:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the arguments ***x*** and ***y***.  
:fontawesome-solid-circle-exclamation: Argument ***x*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: Argument ***y*** must be **intialized** and **non [NULL.](/C99_Data/Types/null).**  
:fontawesome-solid-circle-exclamation: This function is an *implementation detail.*  

## Usage
```C
enum XCBBitGravity gravity = XCBNorthGravity;
int32_t x = 10;             /* x must be intialized  */
int32_t y = 0;              /* y must be intialized */
uint32_t width = 5;
uint32_t height = 5;
uint32_t border_width = 0;

applygravity(gravity, &x, &y, width, height, border_width);

```
