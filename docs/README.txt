Made with Material for MkDocs (https://squidfunk.github.io/mkdocs-material/)


Just want to build the thing?

1. run setup.sh
./setup.sh
2. run build.sh
./build.sh

Want to see how it looks?

1. run run_preview.sh
./run_preview.sh

Want to not have a stroke writing alot?
Take alot at the util/ Folder




This was made using the document generator mkdocs, if you dont known how to use it then learn!
But if you are just quickly trying to document something its simple!

1. run setup.sh
./setup.sh

1. Open the mkdocs.yml in a text editor (vIM/vscode/notepad++, etc..).
vim mkdocs.yml
2. Navigate down to 'nav'
3. If your adding a new section add ' - YourSectionName: ' to the thing.
4. To add a page to that section like what it is or whatever make it and link it say MySectionDirectory/index.md
5. To add stuff under that section just do MySectionDirectory/my_stuff.md
6. To add another section just do the same but just indent it under.


If you dont know how to code and just want to copy the format to help with doc making you can still contribute!
Just open an issue about it and detailing what you wanted to help imrpove with similiar format.
For example

Open Issue

Issue Name: Documentation Client (No code):
Issue description: blah blah blah

Try to make it close to what it would look, if you cant make it look right, then describe what it is suppose to be.
For Example:

Client:
    The client is a structure holding the data for windows
    
    < This is suppose to be a table holding the info for how its suppose to look >
    Type                Name            Description
    integer             x               This is the windows x axis placement.
