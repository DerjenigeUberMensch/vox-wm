# Plans

The following are plans that we expect to have completed before our final release

## Keybinds

Keybind support is something that will be heavily expanded on following v5.0.0+

- Dynamic keybind system in lua and config
- Keybind support for non XGrabKey type keybinds (no required modifier and arbitrary key control)
- The following extra keybind defaults:
    - `Win + L` (external Lock Screen)
    - `Win + ,` (Peek dekstop)
    - `Alt + F4` (Close active window)
    - `PrtScn` (external screenshot)

## Docs

Documentation is currently underway and is planned to be complete by hopefully by v7.0.0 and if not then by v10.0.0 it will be most certaintly completed. Current plans are to use a framework such as MKDocs, to generate it. Handwritten documentation is planned with AI assistance being possible but unlikely to majorily cover docs.

## UI

UI will not be integrated into the wm, this decision may be subject to change, but be considered final as of 10/26/26.

A separate UI application is being considered. It would communicate through standard X11, not requiring a custom IPC interface.

## X11 ICCCM & EWMH Compliance

By v8.0.0, a majority of applicable ICCCM and EWMH specifications will have been implemented. Any remaining unsupported features will be documented as either intentional design decisions or known limitations, that may be adressed by v10.0.0 but are unlikely to be so.

## Complete development

By v10.0.0, vox-wm is expected to be feature complete. Beyond this version, development will primarily focus on maintenance and bug fixes. Major features may still be introduced, but no additional major releases are currently planned.

## Future plans

By v10.0.0 we are considering porting to wayland if certain conditions are met.
