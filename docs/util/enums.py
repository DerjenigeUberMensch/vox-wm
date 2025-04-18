import os










def main():
    # An Example!
    """
    create_enum("ClientFlags", "UNUSED", "The ClientFlagFloating flag is a flag that does xyz", "1u << 0", IMPLEMENTED, False)
    """





















if __name__ == "__main__":
    main()





# Implemnetation

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

    return title + descrip + impl + vl

def create_enum(enum_type_name : str, enum_name : str, decription : str, value : str, implemented : int, used : bool):
    path = os.path.join(os.getcwd(), "Enums", enum_type_name)

    if(not os.path.exists(path)):
        os.makedirs(path)

    file_path = os.path.join(path, enum_name + ".md")

    with open(file_path, "w") as file:
        md_data = get_enum_markdown(enum_type_name, enum_name, decription, value, implemented, used)
        file.write(md_data)




