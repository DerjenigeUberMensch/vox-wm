# Client

## Description

The Client struct is one of the biggest structures in the WM, however it is one of the least used ones in bytes.
In some cases it may be possible to completely neglect dynamic memory and simply use a stack memory of say 10kiB.
That is with all the features though, without easily this could be say ~1kiB or less.
Yet it is still one of the most important structures to know how to use and what not to use.

