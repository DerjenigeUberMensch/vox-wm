# Safebool
The safebool.h header allows for a safe manner to use boolean values 
without breaking compatiblity between versions this is good to use when porting external code and or making the code base portable among codebases.

## #Defines

The followng macros may be in place.

### Defs

The following are defined or re-defined if already defined.

---

```C
#define __bool_true_false_are_defined (1)
```


### Undefs

The following are un-defined if defined.

---

```C
#undef bool  
#undef true  
#undef false  
#undef True  
#undef False  
```

## Types

The followng types may be in place.

### Always

The following may be defined however are guaranteed to exist eitherway.

---

```C
true, True
false, False
bool
```

### Sometimes

The following may be defined however do not have the gurantee to exist in any capacity.

!!! Warning
    These types should not be used directly (Type) however may be used indirectly by using the Types members and or enum values.
    
----

```C
enum boolc;
enum booll;
```
