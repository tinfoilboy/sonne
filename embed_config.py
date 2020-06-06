"""
A simple script to overwrite the config_generator.cpp source file with code for generating a default config.

This script opens the default_config.json file and loads it as a JSON object, then C++ code is generated for creating a
completely default config that is based off of the JSON file.

For running this script, first argument after the script name should be the config file path, and the second should be
the path for the config_generator.cpp file.
"""

import json
import random
import sys
import re
import string

CONFIG_FILE_PATH    = sys.argv[1]
GENERATOR_FILE_PATH = sys.argv[2]

config = {}

with open(sys.argv[1]) as file:
    config = json.load(file)

GENERATOR_FILE_CONTENTS = """/**
This header gets auto-generated by the CMake prebuild step for creating a default configuration.

Do not edit this file by hand! If you want to change the default config, edit the `default_config.json` file.
*/

#include "computare/pch.hpp"

#include "computare/config_generator.hpp"
#include "computare/config.hpp"

using namespace Computare;

std::shared_ptr<Config> Computare::GenerateDefaultConfig()
{
    std::shared_ptr<Config> config = std::make_shared<Config>();

"""

# an indent of four spaces for the lines in the function
FUNCTION_INDENT = "    "

def get_cpp_bool(bool):
    """
    Simply converts a Python boolean value to a string representation of a C++ bool
    """

    return "true" if bool else "false"

if "ignoreHidden" in config:
    GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "config->SetIgnoreHidden({});\n".format(get_cpp_bool(config["ignoreHidden"]))
    
if "blockSize" in config:
    GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "config->SetBlockSize({});\n".format(int(config["blockSize"]))

if "ignore" in config:
    ignoreArray = config["ignore"]

    for ignore in ignoreArray:
        shouldIgnore = (ignore[0] != '!')

        print(ignore)

        # if we should not ignore this path, remove the exclaimation point from the path
        if not shouldIgnore:
            ignore = ignore[1:]

        GENERATOR_FILE_CONTENTS += "\n" + FUNCTION_INDENT + "config->AddIgnored({}, {});".format(ignore, get_cpp_bool(shouldIgnore))

if "languages" in config:
    languages = config["languages"]

    for language in languages:
        GENERATOR_FILE_CONTENTS += "\n" # add extra newline to separate

        # if the language contains any special characters or numbers, just remove them for the variable name
        var_name = re.sub('\W+', '', language["name"])
        var_name = re.sub('\s+', '_', var_name)

        # instantiate the language pointer
        GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "std::shared_ptr<Language> {} = std::make_shared<Language>();\n\n".format(var_name)

        # set the language name in the struct
        GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->name = \"{}\";\n".format(var_name, language["name"])

        # start printing out a vector of extensions for the language to match to
        GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->extensions = {{\n".format(var_name)

        extensionIdx = 0

        extensions = language["extensions"]

        for extension in extensions:
            extensionIdx += 1

            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + FUNCTION_INDENT + "\"{}\"{}\n".format(extension, "," if extensionIdx != len(extensions) else "")

        GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "};\n\n";

        if "lineComment" in language:
            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->lineComment = \"{}\";\n".format(var_name, language["lineComment"])
           
        if "blockCommentBegin" in language:
            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->blockCommentBegin = \"{}\";\n".format(var_name, language["blockCommentBegin"])
           
        if "blockCommentEnd" in language:
            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->blockCommentEnd = \"{}\";\n".format(var_name, language["blockCommentEnd"])

        # if it exists, print out a vector of string delimiters
        if "stringDelimiters" in language:
            delimiters = language["stringDelimiters"]

            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "{}->stringDelimiters = {{\n".format(var_name)

            delimiterIdx = 0

            for delimiter in delimiters:
                delimiterIdx += 1

                if delimiter == "\"":
                    delimiter = "\\\"" # escape the quote for compilation

                GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + FUNCTION_INDENT + "\"{}\"{}\n".format(delimiter, "," if delimiterIdx != len(delimiters) else "")

            GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "};\n";
        
        GENERATOR_FILE_CONTENTS += "\n" # separate final line and add line

        GENERATOR_FILE_CONTENTS += FUNCTION_INDENT + "config->AddLanguage({});\n".format(var_name) # add a new line for the next language or next line

GENERATOR_FILE_CONTENTS += "\n" + FUNCTION_INDENT + "return config;\n"

GENERATOR_FILE_CONTENTS += "}" # finally, end with the closing of the function

with open(sys.argv[2], 'w') as file:
    file.write(GENERATOR_FILE_CONTENTS)

    file.close()