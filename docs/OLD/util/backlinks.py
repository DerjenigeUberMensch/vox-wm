import os
import re
import regex
import argparse

# Function to get all directories in a directory

def GetDirsInDir(Directory, use_absolute_path=False):
    try:
        # Convert the directory to an absolute path if it's relative
        absolute_directory = os.path.abspath(Directory)

        # Check if the directory exists before proceeding
        if not os.path.exists(absolute_directory):
            #print(f"Directory '{absolute_directory}' not found.")
            return []
        
        dirs = [d for d in os.listdir(absolute_directory) if os.path.isdir(os.path.join(absolute_directory, d))]
        
        if use_absolute_path:
            dirs = [os.path.abspath(os.path.join(absolute_directory, d)) for d in dirs]
        
        return dirs
    except FileNotFoundError:
        #print(f"Directory '{Directory}' not found.")
        return []


# Function to get all files in a directory
def GetFilesInDir(Directory, use_absolute_path=False):
    try:
        # Convert the directory to an absolute path if it's relative
        absolute_directory = os.path.abspath(Directory)

        # Check if the directory exists before proceeding
        if not os.path.exists(absolute_directory):
            #print(f"Directory '{absolute_directory}' not found.")
            return []
        
        files = [f for f in os.listdir(absolute_directory) if os.path.isfile(os.path.join(absolute_directory, f))]
        
        if use_absolute_path:
            files = [os.path.abspath(os.path.join(absolute_directory, f)) for f in files]
        
        return files
    except FileNotFoundError:
        #print(f"Directory '{Directory}' not found.")
        return []

def GetFileName(PATH):
    return os.path.basename(PATH)

def GetLocalDirNameOnly(PATH):
    return os.path.basename(os.path.dirname(PATH))


def GetRelativeBackLink(current_file, target_file):
    return os.path.relpath(target_file, start=os.path.dirname(current_file))




def DocGetEnumFiles(SPECIFIC_DOCK_DIR : str):
    ENUM_DIR = "Enums"
    enum_path = os.path.join(SPECIFIC_DOCK_DIR, ENUM_DIR)
    enums = GetDirsInDir(enum_path, True)
    files = []
    for dir in enums:
        files += GetFilesInDir(dir, True)
    return files

def DocGetFunctionFiles(SPECIFIC_DOCK_DIR : str):
    FUNCTION_DIR = "Functions"
    function_path = os.path.join(SPECIFIC_DOCK_DIR, FUNCTION_DIR)
    functions = GetFilesInDir(function_path, True)
    return functions

def DocGetFileNamesOnly(DOC_FILE_PATHS : list[str]):
    names = []
    for path in DOC_FILE_PATHS:
        names += GetFileName(path)
    return names

def DocRegexEnumPatterns(file_path, enum_path):
    content = ""
    with open(file_path, "r") as file:
        content = file.read()

        enumFileName = GetFileName(enum_path)
        enumLocalDir = GetLocalDirNameOnly(enumFileName)
        enumBackLink = GetRelativeBackLink(file_path, enumFileName)
        enumRegexExprs = [f" enum {enumLocalDir} {enumFileName} ", f" {enumLocalDir} {enumFileName} ", f" {enumFileName} "]

        for enumRegexExpr in enumRegexExprs:
            re.sub(enumRegexExpr, f" [{enumRegexExpr}]({enumBackLink}) ", content)
    return content

def DocRegexFunctionPatterns(file_path, function_path):
    content = ""
    with open(file_path, "r") as file:
        content = file.read()

        functionFileName = GetFileName(function_path)
        functionBackLink = GetRelativeBackLink(file_path, functionFileName)
        # Regex1: Make sure function() is not part of any other word, and that there can be stuff in the ()
        # Regex2: Does the same as above but without the ()
        functionRegexExprs = { r"(\b{0}\s*\(.*\)\b)".format(functionFileName): r"[\1]",  r"(\b{0}\b)".format(functionFileName): r"[\1]" }

        for functionRegexExpr, functionFormat in functionRegexExprs.items():
            regex.sub(functionRegexExpr, functionFormat + f"({functionBackLink})", content)
    return content

def main():
    DOCS = "../docs/"
    DIRS = GetDirsInDir(DOCS, True)

    for dir in DIRS:
        enumFileNames = DocGetEnumFiles(dir)
        functionFileNames = DocGetFunctionFiles(dir)

        files = enumFileNames + functionFileNames
        for file in files:
            for enumFile in enumFileNames:
                enumprint = DocRegexEnumPatterns(file, enumFile)
            for functionFile in functionFileNames:
                funcprint = DocRegexFunctionPatterns(file, functionFile)
        







































# Example of using the functions
if __name__ == "__main__":
    main()
