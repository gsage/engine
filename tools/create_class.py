#!/usr/bin/python
"""
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
"""
import argparse
import os
import re

script_path = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('-C', '--class', dest="class_name", help='Class name', required=True)
parser.add_argument('-M', '--module', dest="module", help='Module folder', required=True)
parser.add_argument('-F', '--folder', help='Folder to store into', default="")
parser.add_argument('-cf', '--create-folders', action='store_true', dest="create_folders")
parser.add_argument('-ht', '--include-template', dest="include_template", default=os.path.join(script_path, "class.h.template"))
parser.add_argument('-ct', '--src-template', dest="src_template", default=os.path.join(script_path, "class.cpp.template"))
parser.add_argument('-of', '--one-folder', dest="one_folder", default=False, action='store_true')

# vars
args = parser.parse_args()
folder_path = args.folder
module_path = args.module

if args.one_folder:
    src_path = include_path = module_path
else:
    include_path = os.path.join(module_path, "include", folder_path)
    src_path = os.path.join(module_path, "src", folder_path)

include_template = None
src_template = None

try:
    # template files
    include_template = open(args.include_template, "r").read()
    src_template = open(args.src_template, "r").read()
except Exception as e:
    print("WARN: failed to read templates \n %s.\n No auto-generation will be used" % e)

def create_if_not_exists(path, force=False):
    if os.path.exists(path):
        return True

    os.makedirs(path) if force else os.mkdir(path)
    return True

def create_file(path, template, **fields):
    if os.path.exists(path):
        print("File %s already exists, skipping" % path)
        return

    if template is not None:
        content = template
        for key, value in fields.items():
            content = re.sub("%%%s%%" % key, value, content)
    else:
        content = ""

    open(path, "w").write(content)
    print("Created %s file" % path)

for path in [include_path, src_path]:
    if not create_if_not_exists(path, args.create_folders):
        print("Failed to create folder %s, you can try with -cf option" % path)
        exit(1)

h_file = os.path.join(folder_path, "%s.h" % args.class_name)

create_file(os.path.join(include_path, "%s.h" % args.class_name), include_template, className=args.class_name)
create_file(os.path.join(src_path, "%s.cpp" % args.class_name), src_template, className=args.class_name, includePath=h_file)
