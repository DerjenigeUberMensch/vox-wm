# X C Bindings Translation Layer (XCB-TRL)
## Format Transition

XCB-TRL is mainly a Xlib-like write of the XCB protocol bindings, some parts have been lightly documented. This was primarily a personal project.


## What is this?
See above, but its essentially a wrapper around the XCB Protocol, that provides (mostly) feature complete of the default protocol (extension support is limited.)

For example if we had `XSync(display, false)` it would simply be rewritten as `XCBSync(display)`, with `XCBSyncf(display)` being `XSync(display, true)`
But thats a edge case, most are clear cut copy and paste from Xlib while still including XCB's cookie_get, reply_get stuff.

