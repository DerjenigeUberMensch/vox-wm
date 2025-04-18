# WTypeFlagNotification

Indicates that a client is a notification.

These windows are usually override-redirect however,
if not should be treated as temporarily as above all windows including dock/bar like windows.

And deleted after some predetermined amount of time by developer discretion or handled some other way.

There is no recommendation for this type of window, developer discretion is advised.

- [ ] **Not implemented.**

#### Atom
`_NET_WM_WINDOW_TYPE_NOTIFICATION` indicates a notification. 

An example of a notification would be a bubble appearing with informative text such as "Your laptop is running out of power" etc. 

This property is typically used on override-redirect windows.

#### Value
```C
enum 
EWMHFlags
{
    ...,
    WTypeFlagNotification = 1u << 10,
    ...,
};
```
