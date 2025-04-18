# These Docs

When traversing these docs you may find unsual symbols being used and my not known what they mean.

This explains those symbols that may be ambigious.

## Symbols

### ***VLT***
This indicates that a feature, function, enumerator, type, and or other is not implemented in a stable manner,
this means that the implementation may result in unexpected or *undefined* behaviour. 
In which cases the specified implemenation should not used, this is mostly a reminant of past, present, or future implementation and or priority shifting,
when implementors leave code a way that should really be fixed, but are not important enough to be fixed, and or are more of
compliance issues rather than actual ussability issues.

### ***NCST***
This means that a feature, function, enumerator, type and or other is not constant, and compiler specific.
This means that the implementation does not have a constant value and may change during the course of its lifetime, as thus should not be used with 100% certainty of a value.
As such should use the variable/value/function, etc... to retrieve such value, and not a predefined numerator and or object.

In general this means that:  
This value/objet may change at anything though is unlikely to do so.


## Interpretation

### Macros

Macros in the [vox-wm](https://github.com/DerjenigeUberMensch/vox-wm) project are a little different in that they usually arent fully macros.  
You see what the [vox-wm](https://github.com/DerjenigeUberMensch/vox-wm) project considers macros are any function and or macro that is CapitalCase.  

What does that mean?  
Basically that if you have a function that is all caps for example MY_COOL_FUNCTION() or MYSWAGFUNCTION() it is automatically assumed to be a macro. 

Now using this logic you may think that if you had a macro that wasnt all caps like #define Debug(x, ....) it would be rational to believe that it should be treated as if it were a function, yes you are exactly right.

So what did we learn?

```C
/* This is not a macro */
#define NoCapsThing(X)          ((X) + 1)
/* This is a macro */
int CAPS_THING(long X)          { return X + 1; }

/* some more examples */

/* This is a macro */
#define MACRO_FOR_REAL(X)       ((X))
/* This is a macro */
#define MACROREAL(X)            ((X))
/* This is a macro */
void    MACROOFREAL(void)       { (void)0; }
/* This is a macro */
long    MACRO_MACRO(void *x)    { return *(long *)x; }

/* This is not a macro */
#define FakeMacro               ((10))
/* This is not a macro */
#define fakestmacro(X)          ((X))
/* This is not a macro */
int fakefakeMacro(void)         { return 10; }
/* This is not a macro */
int FAKE_FAKE_gamer(void)       { return 0;     }


```


## Layouts
These explain and show layouts and how their represented,

!!! note
    These layouts do not include bar-like windows as they only shrink the window area. 
    (i.e just make the below layouts render as if the bar was part of the screen and not an extra window)

### **Tiled**
The tiled layout is a layout that contains master window(s) in the leftmost area, and regular windows located in the rightmost area.  
Looking something like this:  

=== "1 Master"
    ```
    (1 window in master area)  
    +-----------------+------+  
    |                 |  T1  |  
    |                 +------+  
    |        M1       |  T2  |  
    |                 +------+  
    |                 |  T3  |  
    +-----------------+------+  
    ```
=== "2+ Masters"
    ```
    (2+ windows in master area)   
    +-----------------+------+   
    |                 |  T1  |   
    |        M1       +------+   
    +-----------------+  T2  |   
    |                 +------+   
    |        M2       |  T3  |   
    +-----------------+------+   
    ```
=== "3+ Masters"
    ```
    (2+ windows in master area)   
    +-----------------+------+   
    |        M3       |  T1  |   
    +-----------------+------+   
    |        M2       |  T2  |   
    +-----------------+------+   
    |        M3       |  T3  |   
    +-----------------+------+   
    ```

### **Grid**
The grid layout is a layout that splits windows into grid like chunks,
however prefferring to leave no empty gaps
Looking something like this:

=== "1 window grid"
    ```
    (1 window in master area)  
    +------------------------+
    |                        |
    |                        |
    |          M1            |
    |                        |
    |                        |
    +------------------------+
    ```
=== "2 window grid"
    ```
    (2 window in master area)
    +-----------+------------+       
    |           |            |
    |           |            |
    |    M1     |     M2     |
    |           |            |
    |           |            |
    +-----------+------------+       
    ```
=== "3 window grid"
    ```
    (3 window in master area)        
    +-----------+------------+       
    |           |            |
    |           |     M2     |
    |    M1     +------------+
    |           |            |
    |           |     M3     |
    +-----------+------------+       
    ```
=== "4 window grid"
    ```
    (4 window in master area)        
    +-----------+------------+       
    |           |            |
    |    M1     |     M2     |
    +-----------+------------+
    |           |            |
    |    M3     |     M4     |
    +-----------+------------+       
    ```

### **Monocle**
The Monocle layout, means that floating windows are given priority over docked windows.
See DOCKED for specifications, but in general means that if a window takes up the whole screen, but is not fullscreen, then its DOCKED.
Windows with other priorities are excersied as such in stackpriority().
Other than that no special properties are assigned to windows or window position.

=== "No Floating Windows"

    ```
    (No floating windows) 
    +------------------------+
    |                        |
    |                        |
    |          M1            |
    |                        |
    |                        |
    +------------------------+

    ```
    !!! note
        (There may be an infinite amount of windows (see uint16_t bounds), however wont be show over the currently focused window if they are not floating.)
=== "With Floating Windows"
    ```
    (With floating window(s)) 
    +------------------------+
    |                        |
    +------+      +----------+-------+
    |      |    M1|          |       |
    |  M3  |      |          |       |
    |      |      |        M2|       |
    +------+------+----------+       |
                  |                  |
                  +------------------+
    ```

### **Floating**
The floating layout, meaning that floating windows have no priority, and are treated as normal windows.


=== "With Floating Windows"
    ```
    (With floating window(s)) 
    +------------------------+
    |                        |
    +------+      +----------+-------+
    |      |    M1|          |       |
    |  M3  |      |          |       |
    |      |      |        M2|       |
    +------+------+----------+       |
                  |                  |
                  +------------------+
    ```

See [Monocle](#monocle) for base handling.
