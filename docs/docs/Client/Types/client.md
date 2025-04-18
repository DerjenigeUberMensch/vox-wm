# Client

!!! Note
    Unless otherwise stated pointers are assumed to be Nullable, i.e they can be of value [`NULL`](/C99_Data/Types/null)


| Name     | Age | Occupation     |
|----------|-----|----------------|
| John     | 25  | Developer      |
| Sarah    | 28  | Designer       |
| Michael  | 35  | Manager        |


x       int16_t  The x coordinate.  
y       int16_t  The y coordinate.  
w       uint16_t The width size.   
h       uint16_t The height size.  

oldx    int16_t  The previous x coordinate.
oldy    int16_t  The previous y coordinate.
oldw    uint16_t The previous width size.
oldh    uint16_t The previous height size. 

bw      uint16_t The border width.
oldbw   uint16_t The previous border width.

bcol        uint32_t                The border colour BLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24)

mina        float                   The minimum aspect ratio (.5 == 50%).
maxa        float                   The maximum aspect ratio (.5 == 50%).

basew   uint16_t The base width.
baseh   uint16_t The base height
incw    uint16_t The increment width.
inch    uint16_t The increment height.

maxw    uint16_t The maximum width.
maxh    uint16_t The maximum height.

minw    uint16_t The minimum width.
minh    uint16_t The minimum height.

win         XCBWindow               The Window ID.
pid         pid_t                   The Process ID.

next        Client *                The Next Mapped Client      (use nextclient() to access).
prev        Client *                The Previous Mapped Client  (use prevclient() to access).

snext       Client *                The Next Stack Client.      (use nextstack() to access).
sprev       Client *                The Previous Stack Client.  (use prevstack() to access).

rnext       Client *                The Next Restack Client.    (use nextrstack() to access).
rprev       Client *                The Previous Restack Client.(use prevrstack() to access).

fnext       Client *                The Next Focused Client.    (use nextfocus() to access).
fprev       Client *                The Previous Focused Client.(use prevfocus() to access).

desktop     Desktop *               The Desktop Parent, this is never NULL.
decor       Decoration *            The Decoration (titlebar).

netwmname   char *                  The client name in utf8 (newer version not supported on all windows).
wmname      char *                  The client name in Compound Text (old version should be set for application).
classname   char *                  The class/group name of a application.
instancename char *                 The individual name of a window instance from the class/group,
if not set or set same as classnmae assume main window.

icon        uint32_t *              The icon.   
format:
BLUE:   (0-255)
GREEN:  (0-255)
RED:    (0-255)
ALPHA:  (0-255)
BLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24)
icon[0] = width;
icon[1] = height;

rstacknum   uint16_t                The restack number, for all intents and purposes implementation.
It is used to set the focus order of clients when performing restacking/arrange calculations.
Where the focus order is counted up so smaller equals higher focus priority.
Donot use unless implementer.

flags   uint16_t ... uint64_t       Custom client flags see XYZ for more information.
ewmhflags   uint32_t                Extended Window Manager Hints flags see XYZ for more information.

gravity     enum XCBBitGravity      The windows bit gravity.
This is used to calculate offset of window positioning on client request.

colormap    XCBColormap             The X11 colormap registered to the client.
