# Welcome to the vox-wm documentation

For help interpreting this guide see **[These Docs](./Docs/index.md)**

## Summary

To begin, the vox-wm project is mostly a mess of code that should have been made a tree but wasnt because of the predecesor project [WSWM](https://github.com/DerjenigeUberMensch/WSWM) (WorkStation Window Manager). 

In short the whole structure is a linked list that uses more linked lists to fanagle its way to a workable project, efforts have been made such as implementing nextclient()/prevclient() type functions, though this has not been fully succesful. 

For future reference though please refer to nextclient()/prevclient(); for accessing client information, as this implementation may be changed to a skip list or array.



## Intial Design

The intial design of [vox-wm](https://github.com/DerjenigeUberMensch/vox-wm) was of a basic linked list, and to compare linked lists to each other to make a conclusion in the state of current client object.

### Client

In brief if we look at the current implementation of client we can see a whole bunch of linked list pointer objects, as seen below.

```C


struct
Client
{
    ...;
    Client *next;       /* The next client in list  */
    Client *prev;       /* The previous client      */
    Client *sprev;      /* The prev stack order clnt*/
    Client *snext;      /* The next client in stack */
    Client *rnext;      /* Restack Next             */
    Client *rprev;      /* Restack Prev             */
    Client *fnext;      /* The next focused client  */
    Client *fprev;      /* The previous focused clnt*/
    ...;
};
```

Now each of these linked lists serve their purpose, and are distinctly seperated by their letter prefix, or in 'next' and 'prev' the lack of.  
Speaking of which will be the first thing we will talk about.  


#### Client Map Listing.

**IGNORE** the following, as developer [DerjenigeUberMensch](https://github.com/DerjenigeUberMensch) incorrectly implemented the following.

The 'Client Map Listing', corresponds to the 'next' and 'prev' pointer items in the 'Client' structure and are fundamentally one of the most ***useless*** client pointers that are currently implemented, yes thats right ***useless***.

However the intended purpose atleast required per the [EWMH](https://specifications.freedesktop.org/wm-spec/latest/) spec was to store the mapping order of clients, basically this just stores the order in which clients were 'mapped' (AKA made visible on screen), so like if client 1 was mapped before client 2, the it would be as order 1,2.

These have their corresponding function call to 'add/attach' to a linked list, and are automatically attached to the start of a linked list as the new 'head' using the function `attach()` unless the client has specified to be `override-redirect`

Likewise they can be 'removed/detached' from their corresponding linked list using the function `detach()`

To access these linked lists safely one can use functions for the base/end of the linked lists using `startclient()` or `lastclient()`, both which require a 'desktop' as input.

To access the next/previous of each linked client one *must* use `nextclient()` or `prevclient()` both requiring input of the client to get the next/previous node from.

#### Stacking Linked List.

The 'Stacking' linked list, holds the current (not future) stack order for current desktop the client is currently in. The 'stack' is how each window is to be displayed in the window manager and its order of precedence, basically this just means that each window has a order, and a window like a popup/dialog box is above a regular window like say google chrome or firefox.

These have their corresponding function call to 'add/attach' to a linked list, and are automatically attached to the start of a linked list as the new 'head' using the function `attachstack()` unless the client has specified to be `override-redirect`

Likewise they can be 'removed/detached' from their corresponding linked list using the function `dettachstack()`

To access these linked lists safely one can use functions for the base/end of the linked lists using `startstack()` or `laststack()`, both which require a 'desktop' as input.

To access the next/previous of each linked client one *must* use `nextstack()` or `prevstack()` both requiring input of the client to get the next/previous node from.

#### Re-Stacking Linked List

The 'Re-Stacking' linked list, just holds the past stack order (see above) for the current desktop the client is in, this is technically *useless* but is done for a `restack()` optimizations, allowing for less requests for X11 to be sent, blah blah blah, basically it reduces flickering as the XServer doesnt have to re-render correctly stacked clients. 

"Some speculation however has been made that this causes 'random' flickering every one in a while due to the fact that clients arent re-rendered and the X Server forgets about them" - [DerjenigeUberMensch](https://github.com/DerjenigeUberMensch)

These have their corresponding function call to 'add/attach' to a linked list, and are automatically attached to the start of a linked list as the new 'head' using the function `attachrstack()` unless the client has specified to be `override-redirect`

Likewise they can be 'removed/detached' from their corresponding linked list using the function `dettachrstack()`

To access these linked lists safely one can use functions for the base/end of the linked lists using `startrstack()` or `lastrstack()`, both which require a 'desktop' as input.

To access the next/previous of each linked client one *must* use `nextrstack()` or `prevrstack()` both requiring input of the client to get the next/previous node from.

#### Focus Linked List

The 'Focus' linked list, holds the focus order of the current desktop the client is in, it has no other special properties

These have their corresponding function call to 'add/attach' to a linked list, and are automatically attached to the start of a linked list as the new 'head' using the function `attachfocus()` unless the client has specified to be `override-redirect`

Likewise they can be 'removed/detached' from their corresponding linked list using the function `dettachfocus()`

To access these linked lists safely one can use functions for the base/end of the linked lists using `startfocus()` or `lastfocus()`, both which require a 'desktop' as input.

To access the next/previous of each linked client one *must* use `nextfocus()` or `prevfocus()` both requiring input of the client to get the next/previous node from.

### Desktop

Desktops follow a similiar principle to clients in that they store objects in linked lists, however are vastly more simplified.

As seen below.

```C
struct 
Desktop
{
    ...;
    Client *sel;                /* Selected Client              */
    Desktop *next;              /* Next Client in linked list   */
    Desktop *prev;              /* Previous Client in list      */
    ...;
};

```

#### Selected

The selected client `sel` is for all intensive purposes to be the currently focused client, this feature is not yet deprecated, but plans are made to deprecated this feature as direct access to structures should not be allowed.

Desktops are connected by linked lists connection by 'next' and 'prev' they should not be directly accessed and instead be accessed via `nextdesktop()` and `prevdesktop()`

### Monitor

Monitors contain desktops which are currently manually attached, this is in plans to be reworked to be able to use for general developer use.

#### TODO

