# X C Bindings Translation Layer (XCB-TRL)

XCB-TRL is a thin, **Xlib style wrapper over the XCB** protocol bindings.

Where we tried to make a **1:1 mapping of Xlib to XCB.**

## Why use it?

We made many improvements such as **inlined documentation**, and **flattening of enums**.

## What we did?

There were many **renamings** of **functions** due to their lack of informative design, the **main ones** being the **`_reply`** type functions, see in XCB, when you try and get a reply or a response from the server. You need to send a request ie `xcb_get_property()`, but it isnt always apparent how `xcb_get_property()` and say `xcb_map_window()` are different. As they both return a `xcb_void_cookie_t`, so we made the explicit design choice early on to remove all that, and the rather redundant extra 'types' such as `xcb_get_property_cookie_t`. 

As that isnt really neccesary if you name your function getters correctly. Which is what we did, we change all those `xcb_get_property()` type functions to more explicit `XCBGetPropertyCookie()`, and `XCBGetPropertyReply()`. 

So now you will always know if a cookie is meant to be 'awaited' or replied back. Instead of having to guess based on the return type.


## Scope and Limitations
- Core protocol coverage is largely complete
- Extension support is limited.
- API design favors clarity and Xlib parity over xcb's minimalism
- Thread safety is guaranteed all functions included in XCB-TRL.
- This project is stable.
