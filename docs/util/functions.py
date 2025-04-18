import os

IMPLEMENTED = 0
PARTIAL = 1
NOT_IMPLEMENTED = 2





ENUM_REFERENCES = []





class DocEnum:
    def __init__(self, type_name : str, name : str, description : str, value : str, implemented : int, used : bool):
        self.type : str = type_name
        self.name : str = name
        self.description : str = description
        self.value : str = value
        self.implemented : int = implemented
        self.used : bool = used

def get_enum_markdown(enum_type_name : str, enum_name : str, decription : str, value : str, implemented : int, used : bool):

    title = f"# {enum_type_name}\n\n"
    descrip = f"{decription}\n\n"
    impl = ""

    if(implemented == IMPLEMENTED):
        impl = "- [x] **Currently"
    elif (implemented == PARTIAL):
        impl = "- [x] **Partially"
    elif (implemented == NOT_IMPLEMENTED):
        impl = "- [ ] **Not"
    impl += " Implemented."
    if(not used):
        impl += " (unused)"
    impl += "**\n"

    vl = f"\n#### Value\n```C\nenum\n{enum_type_name}\n"
    vl += "{\n    ...,\n"
    vl += f"    {enum_name} = {value},\n"
    vl += "    ...,\n};\n```\n"

    ENUM_REFERENCES += DocEnum(enum_type_name, enum_name, description, value, implemented, used)

    return title + descrip + impl + vl

def create_enum(enum_type_name : str, enum_name : str, decription : str, value : str, implemented : int, used : bool):
    path = os.path.join(os.getcwd(), "Enums", enum_type_name)

    if(not os.path.exists(path)):
        os.makedirs(path)

    file_path = os.path.join(path, enum_name + ".md")

    with open(file_path, "w") as file:
        md_data = get_enum_markdown(enum_type_name, enum_name, decription, value, implemented, used)
        file.write(md_data)




class Arg:
    def __init__(self, arg_type : str, arg_name : str, arg_description : str, must_be_intialized : bool = False, is_pointer_modifier = False, extra_warnings : list[str] = []):
        self.arg_type : str = arg_type.strip()
        self.arg_name : str = arg_name.strip()
        self.arg_description : str = arg_description.strip()
        self.must_be_intialized = must_be_intialized
        self.is_modifier = is_pointer_modifier
        self.extra_warns = extra_warnings

class Function:

    __functions__ = []

    def __init__(self, section : str, _return : str, _return_description : str, function_name : str, args : list[Arg], description : str = "", usage : str = "", is_implementation_detail : bool = False):
        self.section : str= section
        self._return : str = _return
        self._return_description : str = _return_description
        self.function_name : str = function_name
        self.args : list[Arg] = args
        self.description : str = description
        self.usage : str = usage
        self.is_implementation_detail : bool = is_implementation_detail
        Function.__functions__.append(self)

    @staticmethod
    def Create():
        path = os.path.join(os.getcwd(), "Functions")

        if(not os.path.exists(path)):
            os.makedirs(path)

        for function in Function.__functions__:
            dir_path = os.path.join(path, function.section)
            if(not os.path.exists(dir_path)):
                os.makedirs(dir_path)

            file_path = os.path.join(dir_path, function.function_name + ".md")
            with open(file_path, "w") as file:
                md_data = function.GetMarkdown()
                file.write(md_data)

    def GetMarkdown(self):
        title = f"# {self.function_name}\n\n"
        signature = "## Signature\n\n---\n\n"
        signature_code = f"```C\n\n{self._return}\n{self.function_name}("

        if(len(self.args) == 0):
            signature_code += "\n    void"
            signature_code += "\n    );\n\n```\n\n---\n\n"

        for arg in self.args:
            LAST = self.args[-1]
            _arg_type = arg.arg_type
            # Dont add space if its a pointer * only if regular type
            if("*" not in _arg_type):
                _arg_type += " "
            signature_code += f"\n    {_arg_type}{arg.arg_name}"
            if(arg != LAST):
                signature_code += ", "
            else:
                signature_code += "\n    );\n\n```\n\n---\n\n"
        
        argument = "## Arguments\n"
        arg_table_header = "\n| Arg | Type | Description | \n"
        arg_table_format = "| :--- | :--- | :--- |\n"
        arg_table = ""

        if(len(self.args) == 0):
            arg_table_header = ""
            arg_table_format = ""
            arg_table = "\nThere are no arguments in this function.\n"

        for arg in self.args:
            name = arg.arg_name
            _type = arg.arg_type
            desc = arg.arg_description
            
            _type = _type.replace("*", "\\*")
            desc = desc.replace("*", "\\*")

            arg_table_element = f"| *{name}* | ***{_type}*** | **{desc}** |\n"
            arg_table += arg_table_element 

        _return = "\n## Return\n\n"

        if(len(self._return) != len("void") or "void" not in self._return):
            _return += f"{self._return_description}\n\n"
            _return += f"- [x] This function returns a {self._return}."
        else:
            _return += ":octicons-x-circle-fill-12: This function does not return a value."

        description = "\n\n## Description\n\n"

        BASE_START_DESCRIPTION_STRING = f"The {self.function_name}() function " 
        if(BASE_START_DESCRIPTION_STRING not in self.description):
            description += BASE_START_DESCRIPTION_STRING
        description += self.description


        # Warning stuff

        # Manual

        must_be_intialized = []
        for arg in self.args:
            if(arg.must_be_intialized):
                must_be_intialized.append(arg)

        if(len(must_be_intialized)):
            description += "\n\n:fontawesome-solid-circle-exclamation: This function ***changes*** the value of the argument(s) "
            if(len(must_be_intialized) != 1):
                for arg in must_be_intialized:
                    if(arg == must_be_intialized[-1]):
                        description += f"and ***{arg.arg_name}***"
                    else:
                        description += f"***{arg.arg_name}***, "
            else:
                description += f"***{must_be_intialized[0].arg_name}***"
            description += ".  \n"

        # User specified
        for arg in self.args:
            for warn in arg.extra_warns:
                description += f":fontawesome-solid-circle-exclamation: {warn}  \n"
            if(arg.must_be_intialized):
                description += f":fontawesome-solid-circle-exclamation: Argument ***{arg.arg_name}*** must be **intialized**"
                # Is it a pointer?
                if("*" in arg.arg_type):
                    description += " and **non [NULL](/C99_Data/Types/null)**"

                description += ".  \n"


        # Arg stuff 

        # Replace stuff 
        funcs = []

        for part in description.split():
            if "()" in part:
                if(part.split("()")[0] not in funcs):
                    funcs.append(part.split("()")[0])

        for func in funcs:
            # this resorts to ../ to allow for new links to be open using right click if its the same function
            description = description.replace(f"{func}()", f"[*{func}()*](../{func})")

        for arg in self.args:
            name = arg.arg_name
            description = description.replace(f" {arg.arg_name} ", f" ***{arg.arg_name}*** ")
            description = description.replace(f"'{arg.arg_name}'", f"***'{arg.arg_name}'***")
            description = description.replace(f"[{arg.arg_name}]", f"***[{arg.arg_name}]***")
            description = description.replace(f"({arg.arg_name})", f"***({arg.arg_name})***")
            description = description.replace(f"\"{arg.arg_name}\"", f"***\"{arg.arg_name}\"***")


        # Usage
        usage = f"\n## Usage\n\n```C\n\n{self.usage}\n\n```"

        return title + signature + signature_code + argument + arg_table_header + arg_table_format + arg_table + _return + description + usage




















def main():
    # An Example!
    """
    arg = Arg("void *", "x", "its a void *", False)
    func = Function("Client", "void *", "Used for ABC", "LockInFunction", [arg], "description", "int a = x;\nfor(int i in range; ++i);", False)

    # Pushes all functions to creation, only call once.
    Function.Create()

    create_enum("ClientFlags", "UNUSED", "The ClientFlagFloating flag is a flag that does xyz", "1u << 0", IMPLEMENTED, False)
    """


    arg0 = Arg("Desktop *", "desktop", "A pointer to the desktop object", True, True)
    arg01 = Arg("Client *", "c", "A pointer to the client object", False, True)
    arg = Arg("Client *", "c", "A pointer to the client object", True, True)
    arg1 = Arg("XCBWindowGeometry *", "geometry", "A pointer to the window geometry object.", False)

    tmp1 = ""
    tmp2 = ""
    tmp3 = ""

    func = Function("Client", "void", "", "clientinitdecor", [arg], "The clientinitdecor() function intializes the **c**->***decor*** property.", "Client *c = your_client_ptr;\n\nclientinitdecor(c);", True)

    func = Function("Client", "void", "", "clientinitfloat", [arg], "The clientinitfloat() function intializes and detects whether or not a client is detected, to be a [floating](/Docs/#floating) type window.", "Client *c = your_client_ptr;\n\nclientinitfloat(c);", True)

    func = Function("Client", "void", "", "clientinitgeom", [arg, arg1], "The clientinitgeom() function intializes the clients geometry.", "Client *c = your_client_ptr;\n\nXCBDisplay *display = _wm.dpy;\nXCBWindow window = c->win;    /* client has the window attribute intialized in this case */\nXCBCookie cookie;\nXCBWindowGeometry *wg;\n\ncookie = XCBGetWindowGeometryCookie(display, window);\nwg = XCBGetWindowGeometryReply(display, cookie);\n\nclientinitgeom(c, wg);\n\nfree(wg);", True)


    arg1 = Arg("XCBWindowProperty *", "wtypeprop", "A pointer to the window type object.", False)

    func = Function("Client", "void", "", "clientinitwtype", [arg, arg1], "The clientinitwtype() function intializes the clients geometry.", "Client *c = your_client_ptr;\n\nXCBDisplay *display = _wm.dpy;\nXCBWindow window = c->win;      /* client has the window attribut already intialized in this case */\nXCBCookie cookie;\nXCBGetWindowProperty *wp;\n\nconst uint8_t NO_BYTE_OFFSET = 0;\nconst uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;\n\ncookie = XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, false, XCB_ATOM_ATOM);\nwp = XCBGetWindowPropertyReply(display, cookie);\n\nclientinitwtype(c, wp);\n\nfree(wp);", True)

    arg1 = Arg("XCBWindowProperty *", "wstateprop", "A pointer to the window state object.", False)

    func = Function("Client", "void", "", "clientinitwstate", [arg, arg1], "The clientinitwstate() function intializes the clients window state(s).", "Client *c = your_client_ptr;\n\nXCBDisplay *display = _wm.dpy;\nXCBWindow window = c->win;      /* client has the window attribut already intialized in this case */\nXCBCookie cookie;\nXCBGetWindowProperty *wp;\n\nconst uint8_t NO_BYTE_OFFSET = 0;\nconst uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;\n\ncookie = XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, false, XCB_ATOM_ATOM);\nwp = XCBGetWindowPropertyReply(display, cookie);\n\nclientinitwstate(c, wp);\n\nfree(wp);", True)

    arg1 = Arg("XCBWindow", "trans", "The transient window to intialize client with", False)

    func = Function("Client", "void", "", "clientinittrans", [arg, arg1], "The clientinittrans() function intializes the clients transient state.", "Client *c = your_client_ptr;\n\nXCBDisplay *display = _wm.dpy;\nXCBWindow win = c->win;\nXCBCookie trans_cookie = XCBGetTransientForHintCookie(display, win);\n\nXCBWindow trans;\nuint8_t trans_status = XCBGetTransientForHintReply(display, cookie, &trans);\n/* If no trans make sure intialized as 0 */\nif (!trans_status)\n{\n    trans = 0;\n}\n\nclientinittrans(c, trans);", True)


    func = Function("Client", "void", "", "configure", [arg], "The configure() function notifies windows of changes made from x,y,width,height and or border_width, and it also notifies them that they are currently being managed by the window manager as the ->override_redirect field is set to False.", "Client *c = your_client_ptr;\n\nc->x = 10; /* Note you should never directly set the clients x field, instead use resize() to resize clients, but for this example we will */\n\nconfigure(c);")

    func = Function("Client", "Client *", "Returns a Client * if memory for the client was successfully allocated, a NULL pointer otherwise.", "createclient", [], "The createclient() function returns a empty block of memory that has all data intialized and ready to be used, this include field member c->decor being a allocated chunk of memory.", "Client *c = NULL;\n\nc = createclient();", True)

    func = Function("Client", "void", "", "focus", [arg01], "The focus() function attempts to move focus to the specified client using focusrealize() to determine the next client to focus, however by providing an argument, focus() attempts to use that client whenever possible.", "Client *c = your_client_ptr;\n\nfocus(c); /* Try to focus my client */\n\nfocus(NULL); /* Try to focus the next available client */")

    func = Function("Client", "Client *", "Returns a client pointer to the client that was decided to be considered 'focused' under our managmenet, otherwise NULL if no client was focused.", "focusrealize", [arg01], "The focusrealize() function attemps to find the next available client to focus within our builtin system of focusing clients, and like focus() attempts to use the provided arg c when ever possible. This however does not focus any client directly and must be focused using focus() in order to update the windows state in the XServer. Basically this just updates our internal focus order, but doesnt update the XServer focus order. This can be useful when performing certain stacking optimizations.", "Client *c = your_client_ptr;\n\nc = focusrealize(c);\n\n/* perform optimizations here */\n\nfocus(c);")


    arg1 = Arg("bool", "focused", "Whether or not a client should be or is considered 'focused'", False, False)

    func = Function("Client", "void", "", "grabbuttons", [arg, arg1], "The grabbuttons() function is a archaic remnant of the [dwm](https://dwm.suckless.org/) source translation to xcb. But to be brief this function grabs all the buttons needed to perform user defined callbacks, and to provide for clicking other windows, yes we need this in order to replay the pointer back to the window so external clicks register.", "Client *c = your_client_ptr;\n\ngrabbuttons(c, false);", True)

    func = Function("Client", "void", "", "grabkeys", [], "The grabkeys() function is a archaic remnant of the [dwm](https://dwm.suckless.org/) source translation to xcb. But to be brief this function grabs all the keys needed to perform user defined callbacks. Furthermore it should be called only udring mappingnotifies (key mapping) and at the start of the WM, as such should under no circumstance continue operation if function has failed unless user has specified another form of window manager termination.", "grabkeys();")


    arg1 = Arg("enum KillType", "kill_type", "The kill operation to perform on the specified client")

    func = Function("Client", "void", "", "killclient", [arg, arg1], "The killclient() function sends a signal to the specified client to kill further client operation.", "Client *c = your_client_ptr;\n\n/* Kill the client gracefuly */\nkillclient(c, Graceful);\n/* Kill the client using a safe destroy method */\nkillclient(c, Safedestroy);\n/* Kill the client using a hard destroy method */\nkillclient(c, Destroy);\n/* ... (Left for future implementations) */")



    func = Function("Client", "Client *", "Returns the last mapped client in the desktop specified if applicable, otherwise a NULL ptr is returned", "lastclient", [arg0], "The lastclient() function gets the last mapped client in desktop if any are available.", "Desktop *desk = your_desktop_ptr;\nClient *last_mapped_client;\n\nlast_mapped_client = lastclient(desk);")

    func = Function("Client", "Client *", "Returns the last focused client in the desktop specified if applicable, otherwise a NULL ptr is returned", "lastfocus", [arg0], "The lastfocus() function gets the last focused client in desktop if any are available.", "Desktop *desk = your_desktop_ptr;\nClient *last_focused_client ;\n\nlast_focused_client = lastfocus(desk);")

    func = Function("Client", "Client *", "Returns the last 'stack' client in the desktop specified if applicable, otherwise a NULL ptr is returned", "laststack", [arg0], "The laststack() function gets the last 'stack' client in desktop if any are available.", "Desktop *desk = your_desktop_ptr;\nClient *last_stack_client ;\n\nlast_stack_client = laststack(desk);")

    func = Function("Client", "Client *", "Returns the last 'restack' client in the desktop specified if applicable, otherwise a NULL ptr is returned", "lastrstack", [arg0], "The lastrstack() function gets the last 'restack' client in desktop if any are available.", "Desktop *desk = your_desktop_ptr;\nClient *last_rstack_client ;\n\nlast_rstack_client = lastrstack(desk);")

    arg1 = Arg("void **", "replies", "The replybacks from managereplies() to free", True, False, ["Replies are freed after this function returns."])

    func = Function("Client", "void", "", "managecleanup", [arg1], "The managecleanup() function frees any and all resulting data from the replybacks generated from managereplies() as such should ***always*** be used ***after*** calling managereplies().", "XCBWindow win = your_window_id;\nXCBCookie requests[ManageClientLAST];\nvoid *replies[ManageClientLAST];\n\nmanagerequest(win, requests);\nmanagereplies(requests, replies);\n\nmanagecleanup(replies);", True)


    tmp1 = Arg("XCBWindow", "win", "The id to the window to request info from for manage()", False)
    tmp2 = Arg("XCBCookie *", "requests", "The requests to fill back.", False)

    func = Function("Client", "void", "", "managerequest", [ tmp1, tmp2 ], "The managerequest() function requests the XServer for properties relating to the intial management of a window, and fill such data in the correponding field 'requests' ", "XCBCookie requests[ManageClientLAST];\n\nXCBWindow win = your_window_id;\n\nmanagerequest(win, requests);", True)

    tmp1 = Arg("XCBCookie *", "requests", "The requests for managing the client, from managerequest()")
    tmp2 = Arg("void *", "replies", "The replies to fillback unto callee.")

    func = Function("Client", "void", "", "managereplies", [ tmp1, tmp2 ], "The managereplies() function requests for the replyback from the XServer for properties relating to the intial management of a window, and fills such data in the corresponding field, 'replies'", "XCBCookie request[ManageClientLAST];\nvoid *replies[ManageClientLAST];\nXCBWindow win = your_window_id;\n\nmanagrequest(win, requests);\n\nmanagereplies(requests, replies);", True)


    tmp1 = Arg("XCBWindow", "win", "The id to the window for the window manager to manage", False)
    tmp2 = Arg("void *", "replies", "The replies to use when managing the window.")

    func = Function("Client", "Client *", "Returns a Client * if the window specified and argument field 'replies' was managed successfully and is now in the management of the window manager, other wise a NULL ptr will be returned, in which likely one of the following. \n\n 1. The window is a 'root' window\n2. The window is already managed.\n3. The window is override redirect. 4.\n A memory allocation failed.", "manage", [ tmp1, tmp2 ], "The manage() function attempts to manage the window provided if successfull it will treat the window as a Client for the foreseeable future until the window is unmapped/destroyed and or the window becomes override_redirect. Managing a window provides a user the ability to interact with the window, by use of keybinds and toggles, see toggle.h, further more it allows for better management of the window as it can more easily adhere to the window managers protocols.", "Client *c;\nXCBCookie request[ManageClientLAST];\nvoid *replies[ManageClientLAST];\nXCBWindow win = your_window_id;\n\nmanagrequest(win, requests);\nmanagereplies(requests, replies);\n\nc = manage(win, replies);")

    func = Function("Client", "void", "", "maximize", [arg], "The maximize() function maximizes the client to fit the screen conforming to strut boundaries of the bar and the boundaries of the screen in the x/y axis.", "Client *c = your_client_ptr;\n\nmaximize(c);")

    func = Function("Client", "void", "", "maximizehorz", [arg], "The maximizehorz() function maximizes the client to fit the screen conforming to strut boundaries of the bar and the boundaries of the screen in the x axis.", "Client *c = your_client_ptr;\n\nmaximizehorz(c);")

    func = Function("Client", "void", "", "maximizevert", [arg], "The maximizevert() function maximizes the client to fit the screen conforming to strut boundaries of the bar and the boundaries of the screen in the y axis.", "Client *c = your_client_ptr;\n\nmaximizevert(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a mapped order client after this client, otherwise a NULL ptr is returned.","nextclient", [arg], "The nextclient() function gets the next available mapping order client.", "Client *next_client = your_client_ptr;\n\nnext_mapped_client = nextclient(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a stack order client after this client, otherwise a NULL ptr is returned.","nextstack", [arg], "The nextstack() function gets the next available stack order client.", "Client *next_stack_client = your_client_ptr;\n\nnext_stack_client = nextstack(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a restack order client after this client, otherwise a NULL ptr is returned.","nextrstack", [arg], "The nextrstack() function gets the next available restack order client.", "Client *next_restack_client = your_client_ptr;\n\nnext_restack_client = nextrstack(c);")
    
    func = Function("Client", "Client *", "Returns a Client * if there is a focus order client after this client, otherwise a NULL ptr is returned.","nextfocus", [arg], "The nextfocus() function gets the next available focus order client.", "Client *next_focused_client = your_client_ptr;\n\nnext_focused_client = nextfocus(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a tile order client after this client, otherwise a NULL ptr is returned.","nexttiled", [arg], "The nexttiled() function gets the next available tiling order client.", "Client *next_focused_client = your_client_ptr;\n\nnext_tiled_client = nexttiled(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a visible order client after this client, otherwise a NULL ptr is returned.","nextvisible", [arg], "The nextvisible() function gets the next available visible order client.", "Client *next_visible_client = your_client_ptr;\n\nnext_visible_client = nextvisible(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a mapped order client before this client, otherwise a NULL ptr is returned.","prevclient", [arg], "The prevclient() function gets the previous available mapping order client.", "Client *prev_client = your_client_ptr;\n\nprev_mapped_client = prevclient(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a stack order client before this client, otherwise a NULL ptr is returned.","prevstack", [arg], "The prevstack() function gets the previous available stack order client.", "Client *prev_stack_client = your_client_ptr;\n\nprev_stack_client = prevstack(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a restack order client before this client, otherwise a NULL ptr is returned.","prevrstack", [arg], "The prevrstack() function gets the previous available restack order client.", "Client *prev_restack_client = your_client_ptr;\n\nprev_restack_client = prevrstack(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a focus order client before this client, otherwise a NULL ptr is returned.","prevfocus", [arg], "The prevfocus() function gets the previous available focus order client.", "Client *prev_focused_client = your_client_ptr;\n\nprev_focused_client = prevfocus(c);")

    func = Function("Client", "Client *", "Returns a Client * if there is a visible order client before this client, otherwise a NULL ptr is returned.","prevvisible", [arg], "The prevvisible() function gets the previous available visible order client.", "Client *prev_visible_client = your_client_ptr;\n\nprev_visible_client = prevvisible(c);")


    tmp1 = Arg("int32_t", "x", "The new client x coordinate.")
    tmp2 = Arg("int32_t", "y", "The new client y coordinate.")
    tmp3 = Arg("int32_t", "width", "The new client width.")
    tmp4 = Arg("int32_t", "height", "The new client height.")
    tmp5 = Arg("bool", "interact", "Whether or not the client should be confined within the monitor during resize() operations.")

    func = Function("Client", "void", "", "resize", [arg, tmp1, tmp2, tmp3, tmp4, tmp5], "The resize() function attempts to change a clients x/y coordinates and or width/height size. This change makes sure to be compliant with client's requested min/max size and or certain resizing restrictions imposed by the client. See applysizehints() for information on client compliant requests. See resizeclient() on storage and direct X11 changes for client's x, y, width, and or height changes.", "Client *c = your_client_ptr;\nint32_t x = 10;\nint32_t y = 20;\nint32_t width = 500;\nint32_t height = 500;\nbool confine_within_monitor = true\n\nresize(c, x, y, w, h, confine_within_monitor);")



    tmp1 = Arg("int16_t", "x", "The new client x coordinate.")
    tmp2 = Arg("int16_t", "y", "The new client y coordinate.")
    tmp3 = Arg("int16_t", "width", "The new client width.")
    tmp4 = Arg("int16_t", "height", "The new client height.")


    func = Function("Client", "void", "", "resizeclient", [arg, tmp1, tmp2, tmp3, tmp4], "The resizeclient() function changes the dimentions of a client via the specified x, y, width and or height. This function should be noted as not having any security checks and may have hugely offset x and or y coordinates set, width possible negative window width/height, (this breaks windows do not do!). Furthermore this function has a special method of resizing as it does not 'apply' changes if the client is not visible, see ISVISIBLE() for details.", "Client *c = your_client_ptr;\nint16_t x = 0;\nint16_t y = 0;\nint16_t width = 50;\nint16_t height = 50;\n\nresizeclient(c, x, y, w, h);", True)

    tmp3 = Arg("bool", "interact", "Whether or not the client should be confined within the monitor during resize() operations.")

    func = Function("Client", "void", "", "resizemove", [arg, tmp1, tmp2, tmp3], "The resizemove() function attempts to change a clients x/y coordinates. This change makes sure to be compliant with client's requested position restrictions imposed by the client. See applysizehints() for information on client compliant requests. See resizeclient() on storage and direct X11 changes for client's x, y, changes.", "Client *c = your_client_ptr;\nint32_t x = 10;\nint32_t y = 20;\nbool confine_within_monitor = true\n\nresizemove(c, x, y, confine_within_monitor);")


    tmp1 = Arg("XCBAtom", "protocol", "The WMProtocol event to send.")

    func = Function("Client", "void", "", "sendprotocolevent", [arg, tmp1], "The sendprotocolevent() function sends a ClientMessage to the specified client, with the WMProtocol being sent as the type and argument 'protocol' being the sent atom type.", "Client *c = your_client_ptr;\nXCBAtom takefocus = wmatom[WMTakeFocus];\nXCBAtom saveyourself = wmatom[WMSaveYourself];\nXCBAtom deletewindow = wmatom[WMDeleteWindow];\n\n/* Tells client that they are currently being focused */\nsendprotocolevent(c, takefocus);\n/* Tells client to save all its settings (deprecated) */\nsendprotocolevent(c, saveyourself);\n/* Tells a client to 'kill/terminate' itself, i.e delete window*/\nsendprotocolevent(c, deletewindow);")

    tmp1 = Arg("bool", "state", "The current bool state of the flag, true being on/active, false being off/disabled.")

    func = Function("Client", "void", "", "setalwaysontop", [arg, tmp1], "The setalwaysontop() function sets the clients flag state for the enum WStateFlagAbove. This flag signifies to the window manager that clients should be always above others. Usually reserved for picture-in-picture type windows, and or splash windows.", "Client *c = your_client_ptr;\nbool my_flag_state = false;\n\nsetalwaysontop(c, my_flag_state);")

    func = Function("Client", "void", "", "setalwaysonbottom", [arg, tmp1], "The setalwaysonbottom() function sets the clients flag state for the enum WStateFlagBelow. This flag signifies to the window manager that clients should be always below others. Usually reserved for widget type windows, such as 'conky' and some background display windows.", "Client *c = your_client_ptr;\nbool my_flag_state = false;\n\nsetalwaysonbottom(c, my_flag_state);")

    tmp2 = Arg("uint8_t", "alpha", "The opacity level 0-255 of the Client border.")

    func = Function("Client", "void", "", "setborderalpha", [arg, tmp2], "The setborderalpha() function sets the clients border opacity, i.e alpha from a range of 0-255.", "Client *c = your_client_ptr;\nint8_t no_opacity = UINT8_MAX;\n\nsetborderalpha(c, no_opacity);")


    tmp3 = Arg("uint8_t", "red", "The red (ARGB) color level 0-255 of the Client border.")
    tmp4 = Arg("uint8_t", "green", "The green (ARGB) color level 0-255 of the Client border.")
    tmp5 = Arg("uint8_t", "blue", "The blue (ARGB) color level 0-255 of the Client border.")

    func = Function("Client", "void", "", "setbordercolor", [arg, tmp3, tmp4, tmp5], "The setbordercolor() function sets the clients border color using the provided RGB values.", "Client *c = your_client_ptr;\nuint8_t red = 100;\nuint8_t green = 50;\nuint8_t blue = 255;\n\nsetbordercolor(c, red, green, blue);")

    tmp3 = Arg("uint32_t", "argb_color", "The ARGB color shift to be used.")

    func = Function("Client", "void", "", "setbordercolor32", [arg, tmp3], "The setbordercolor32() function sets the clients border color using the provided ARGB value shift, and is formated as follows:  \nBLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24)", "Client *c = your_client_ptr;\nuint8_t red = 100;\nuint8_t green = 50;\nuint8_t blue = 255;\nuint8_t alpha = 255;\nuint32_t bordercol32 = blue + (green << 8) + (red << 16) + (alpha << 24);\n\nsetbordercolor32(c, bordercol32);", True)

    tmp3 = Arg("uint16_t", "border_width", "The border width size.")

    func = Function("Client", "void", "", "setborderwidth", [arg, tmp3], "The setborderwidth() function sets the clients border width. See enum ClientFlagDisableBorder for usage if disabled", "Client *c = your_client_ptr;\nuint16_t border_width = 15;\n\nsetborderwidth(c, border_width);")


    func = Function("Client", "void", "", "setclientdesktop", [arg, arg0], "The setclientdesktop() functions changes the current clients correponding desktop, and or adds it if not already set. However does not visually restack only locally is this change set.", "Client *c = your_client_ptr;\nDesktop *desk = new_desktop_ptr;\n\n/* Note: Client is only internally updated */\nsetclientdesktop(c, desk);\n/* Update externally the changes we made (optional).\n Note for this example it only uses showhide() however in general it would be recommended to use arrange() on both desktops as to maintain stack order. */\nshowhide(c);", True)

    # Change to XCB state thing 
    tmp1 = Arg("uint8_t", "state", "The WMState to set the specified client.")

    func = Function("Client", "void", "", "setclientstate", [arg, tmp1], "The setclientstate() function tells the XServer and the correponding client to change its wmatom[WMState] value to the one provided in argument 'state'", "Client *c = your_client_ptr;\nuint8_t state = XCB_WINDOW_NORMAL_STATE;\n\nsetclientstate(c, state);")

    tmp1 = Arg("XCBAtom", "atom", "The atom to add/remove WType (Window Type) from.")
    tmp2 = Arg("bool", "state", "Whether or not to add/remove an atom, true being to add the atom specified, false being to remove it.")

    func = Function("Client", "void", "", "setclientwtype", [arg, tmp1, tmp2], "The setclientwtype() function notifies the Prop queue, via the PropListenArg function, to update the specified argument 'c' window type atom and its current provided state.", "Client *c = your_client_ptr;\nXCBAtom atom = netatom[NetWMWindowTypeDialog];\nbool add_atom = true;\n\nsetclientwtype(c, atom, add_atom);")

    func = Function("Client", "void", "", "setclientnetstate", [arg, tmp1, tmp2], "The setclientnetstate() function notifies the Prop queue, via the PropListenArg function, to update the specified argument 'c' NetWMState atom and its current provided state.", "Client *c = your_client_ptr;\nXCBAtom atom = netatom[NetWMStateAbove];\nbool add_atom = true;\n\nsetclientwtype(c, atom, add_atom);")

    tmp2 = Arg("bool", "is_border_disable", "Whether or not to disable the current border")

    func = Function("Client", "void", "", "setdisableborder", [arg, tmp2], "The setdisableborder() function sets the current visibility state of the client border if it is not 0, in which case this function would not visibly do anything.", "Client *c = your_client_ptr;\nbool disable_border = true;\n\nsetdisableborder(c, disable_border);")

    tmp2 = Arg("pid_t", "pid", "The Proccess IDentification number to set for client.")

    func = Function("Client", "void", "", "setclientpid", [arg, tmp2], "The setclientpid() function sets Process IDentification (PID) number for the specified client.", "Client *c = your_client_ptr;\npid_t pid = MY_PID;\n\nsetclientpid(c, pid);")

    tmp2 = Arg("enum WMMapState", "state", "The WMMapState to set for client.")

    func = Function("Client", "void", "", "setmapstate", [arg, tmp2], "The setmapstate() function sets the WMMapState for a client, that being mapped and or unmmaped.", "Client *c = your_client_ptr;\nenum WMMapState newState = WMMapStateMapped;\n\nsetmapstate(c, newState);")

    tmp2 = Arg("bool", "state", "The current bool state of the flag, true being on/active, false being off/disabled.")

    func = Function("Client", "void", "", "setwtypedesktop", [arg, tmp2], "sets the WTypeFlagDesktop bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_desktop = true;\n\nsetwtypedesktop(c, is_wtype_desktop);")

    func = Function("Client", "void", "", "setwtypedialog", [arg, tmp2], "sets the WTypeFlagDialog bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_dialog = true;\n\nsetwtypedialog(c, is_wtype_dialog);")

    func = Function("Client", "void", "", "setwtypedock", [arg, tmp2], "sets the WTypeFlagDock bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_dock = true;\n\nsetwtypedock(c, is_wtype_dock);")

    func = Function("Client", "void", "", "setwtypetoolbar", [arg, tmp2], "sets the WTypeFlagToolbar bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_toolbar = true;\n\nsetwtypetoolbar(c, is_wtype_toolbar);")

    func = Function("Client", "void", "", "setwtypemenu", [arg, tmp2], "sets the WTypeFlagMenu bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_menu = true;\n\nsetwtypedmenu(c, is_wtype_menu);")

    func = Function("Client", "void", "", "setwtypeneverfocus", [arg, tmp2], "sets the WStateFlagNeverFocus bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_neverfocus = true;\n\nsetwtypeneverfocus(c, is_wstate_neverfocus);")

    func = Function("Client", "void", "", "setwtypeutility", [arg, tmp2], "sets the WTypeFlagUtility bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_utility = true;\n\nsetwtypeutility(c, is_wtype_utility);")

    func = Function("Client", "void", "", "setwtypesplash", [arg, tmp2], "sets the WTypeFlagSplash bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_splash = true;\n\nsetwtypesplash(c, is_wtype_splash);")

    func = Function("Client", "void", "", "setwtypepopupmenu", [arg, tmp2], "sets the WTypeFlagPopupMenu bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_popup_menu = true;\n\nsetwtypepopupmenu(c, is_wtype_popup_menu);")

    func = Function("Client", "void", "", "setwtypetooltip", [arg, tmp2], "sets the WTypeFlagTooltip bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_tooltip = true;\n\nsetwtypetooltip(c, is_wtype_tooltip);")

    func = Function("Client", "void", "", "setwtypenotification", [arg, tmp2], "sets the WTypeFlagNotification bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_notification = true;\n\nsetwtypenotification(c, is_wtype_notification);")

    func = Function("Client", "void", "", "setwtypecombo", [arg, tmp2], "sets the WTypeFlagCombo bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_combo = true;\n\nsetwtypecombo(c, is_wtype_combo);")

    func = Function("Client", "void", "", "setwtypednd", [arg, tmp2], "sets the WTypeFlagDnd bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_dnd = true;\n\nsetwtypednd (c, is_wtype_dnd);")

    func = Function("Client", "void", "", "setwtypenormal", [arg, tmp2], "sets the WTypeFlagNormal bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_normal = true;\n\nsetwtypednormal(c, is_wtype_normal);")

    func = Function("Client", "void", "", "setwtypemapiconic", [arg, tmp2], "sets the WStateFlagMapIconic bit flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wtype_mapping_iconic = true;\n\nsetwtypedesktop(c, is_wtype_mapping_iconic);")

    func = Function("Client", "void", "", "setwtypemapnormal", [arg, tmp2], "sets the reverse of WStateFlagMapIconic flag for field `ewmhflags` in the client. Due to the fact that vox-wm does not handle withdrawn windows, this bit is reused", "Client *c = your_client_ptr;\nbool is_wtype_mapping_normal = true;\n\nsetwtypemapnormal(c, is_wtype_mapping_normal);")

    func = Function("Client", "void", "", "setwmtakefocus", [arg, tmp2], "sets the WStateFlagWMTakeFocus flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_takefocus = true;\n\nsetwmtakefocus(c, is_wstate_takefocus);")

    func = Function("Client", "void", "", "setwmsaveyourself", [arg, tmp2], "sets the WStateFlagWMSaveYourself flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_save_yourself = true;\n\nsetwmsaveyourself(c, is_wstate_save_yourself);")

    func = Function("Client", "void", "", "setwmdeletewindow", [arg, tmp2], "sets the WStateFlagWMDeleteWindow flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_delete_window = true;\n\nsetwmdeletewindow(c, is_wstate_delete_window);")

    func = Function("Client", "void", "", "setskippager", [arg, tmp2], "sets the WStateFlagSkipPager flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_skip_pager = true;\n\nsetskippager(c, is_wstate_skip_pager);")

    func = Function("Client", "void", "", "setskiptaskbar", [arg, tmp2], "sets the WStateFlagSkipTaskbar flag for field `ewmhflags` in the client.", "Client *c = your_client_ptr;\nbool is_wstate_skip_taskbar = true;\n\nsetskiptaskbar(c, is_wstate_skip_taskbar);")

    func = Function("Client", "void", "", "setshowdecor", [arg, tmp2], "Sets the ClientFlagShowDecor flag for field `flags` in the client and updates `_NET_FRAME_EXTENTS`. " "Currently, this function forces decorations to be disabled for NetWM compliance, but it is not fully implemented yet.", "Client *c = your_client_ptr;\nbool is_wstate_show_decor = true;\n\nsetshowdecor(c, is_wstate_show_decor);")

    func = Function("Client", "void", "", "setfullscreen", [arg, tmp2], "Toggles fullscreen mode by updating EWMH flags and border width.", "Client *c = your_client_ptr;\nbool is_wstate_fullscreen = true;\n\nsetfullscreen(c, is_wstate_fullscreen);")

    func = Function("Client", "void", "", "setfloating", [arg, tmp2], "Toggles floating state for a client.", "Client *c = your_client_ptr;\nbool is_wstate_floating = true;\n\nsetfloating(c, is_wstate_floating);")

    func = Function("Client", "void", "", "setfocus", [arg], "Sets input focus to the client and updates EWMH focus state.", "Client *c = your_client_ptr;\n\nsetfocus(c);")

    func = Function("Client", "void", "", "sethidden", [arg, tmp2], "Sets the WStateFlagHidden flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_hidden = true;\n\nsethidden(c, is_wstate_hidden);")

    func = Function("Client", "void", "", "setkeepfocus", [arg, tmp2], "Sets the ClientFlagKeepFocus flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_keep_focus = true;\n\nsetkeepfocus(c, is_wstate_keep_focus);")

    func = Function("Client", "void", "", "setmaximizedvert", [arg, tmp2], "Sets the WStateFlagMaximizedVert flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_maximized_vert = true;\n\nsetmaximizedvert(c, is_wstate_maximized_vert);")

    func = Function("Client", "void", "", "setmaximizedhorz", [arg, tmp2], "Sets the WStateFlagMaximizedHorz flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_maximized_horz = true;\n\nsetmaximizedhorz(c, is_wstate_maximized_horz);")

    func = Function("Client", "void", "", "setshaded", [arg, tmp2], "Sets the WStateFlagShaded flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_shaded = true;\n\nsetshaded(c, is_wstate_shaded);")

    func = Function("Client", "void", "", "setmodal", [arg, tmp2], "Sets the WStateFlagModal flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_modal = true;\n\nsetmodal(c, is_wstate_modal);")

    func = Function("Client", "void", "", "setoverrideredirect", [arg, tmp2], "Sets the ClientFlagOverrideRedirect flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_override_redirect = true;\n\nsetoverrideredirect(c, is_wstate_override_redirect);")

    func = Function("Client", "void", "", "setsticky", [arg, tmp2], "Sets the WStateFlagSticky flag for the client.", "Client *c = your_client_ptr;\nbool is_wstate_sticky = true;\n\nsetsticky(c, is_wstate_sticky);")

    func = Function("Client", "void", "", "seturgent", [arg, tmp2], "Sets the WStateFlagDemandAttention flag and updates WM hints for urgency.", "Client *c = your_client_ptr;\nbool is_wstate_urgent = true;\n\nseturgent(c, is_wstate_urgent);")














    Function.Create()





if __name__ == "__main__":
    main()




