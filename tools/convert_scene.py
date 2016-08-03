"""
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

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

#!/usr/bin/python
import json
import xml.etree.ElementTree as ET
import argparse
import re

def create_arg_parser():
    parser = argparse.ArgumentParser(description="Convert DotScene file to rsage json")
    parser.add_argument("-i", "--input-file", dest="input_file", help="Dot scene file to convert", required=True)
    parser.add_argument("-o", "--output-file", dest="output_file", help="Json scene file path", required=True)
    parser.add_argument("-p", "--pretty", dest="pretty", action="store_true",
            default=False, help="Prettify output json")
    parser.add_argument("-db", "--dump-bson", dest="dump_bson",
            action="store_true", help="Use bson dumper")
    parser.add_argument("-v", "--verbose", dest="verbose", action="store_true", help="Verbose output")
    return parser

mappings = {}
def convertor(name):
    def wrap(f):
        def wrapped(*args, **kwargs):
            return f(*args, **kwargs)
        mappings[name] = wrapped
        return wrapped
    return wrap

logging_enabled = False
def log(message):
    if logging_enabled:
        print message

class DotSceneData(object):

    def __init__(self):
        self.output = {"environment": {}}
        self.node_tree = {};


    def add_node(self, name, node, parent=None):
        parent_node = self.node_tree
        if parent is not None:
            parent_node = self.node_tree[parent]

        parent_node[name] = node


    def add_entity(self, value):
        if "entities" not in self.output:
            self.output["entities"] = []

        self.output["entities"].append(value)

    def get_entity(self, entity_id):
        if "entities" not in self.output:
            return None

        matched = filter(lambda item: item.get("id") == entity_id, self.output["entities"])
        if not matched:
            return None
        else:
            return matched[0]

    def add_environment_setting(self, key, value):
        self.output["environment"][key] = value

    def get_environment_setting(self, key):
        return self.output["environment"].get(key)

class DotSceneParser(object):
    def __init__(self):
        self.convertors = {}

    def decode(self, file_name):
        tree = ET.parse(file_name)
        root = tree.getroot()
        data = DotSceneData()
        self.iterate_recoursively(data, root);
        return data

    def iterate_recoursively(self, data, node, parent=None):
        node.parent = parent
        tag_name = node.tag.strip()
        if tag_name in mappings:
            mappings[tag_name](data, node)

        for child in node:
            self.iterate_recoursively(data, child, node)

# --------------------------------------------------------------------------------
# convertors
# --------------------------------------------------------------------------------

def get_light_object(node):
    return {
            "name": node.attrib["name"],
            "type": node.attrib["type"],
            "castShadows": bool(node.attrib.get("castShadows", False)),
            "colourDiffuse": get_colour_value(node, "colourDiffuse"),
            "colourShadow": get_colour_value(node, "colourShadow"),
            "colourSpecular": get_colour_value(node, "colourSpecular"),
            "direction": get_vector3(node, "directionVector"),
            "position": get_vector3(node, "position")
           }

@convertor("scenemanager")
def scene_manager_convertor(data, node):
    data.add_environment_setting("sceneManager",
            {
                "name": node.attrib["name"],
                "type": node.attrib["type"],
            })

@convertor("environment")
def colour_ambient_convertor(data, node):
    for node_name in ["colourAmbient", "colourDiffuse", "colourBackground"]:
        data.add_environment_setting(node_name, get_colour_value(node, node_name))

@convertor("fog")
def fog_convertor(data, node):
    data.add_environment_setting("fog",
            {
                "mode": node.attrib.get("mode", "none"),
                "start": float(node.attrib.get("start", 0)),
                "end": float(node.attrib.get("end", 0)),
                "density": float(node.attrib.get("density", 0)),
            })

@convertor("viewport")
def viewport_convertor(data, node):
    data.add_environment_setting("viewport",
            {
                "name": node.attrib["name"],
                "index": node.attrib.get("index", 1),
                "colour": get_colour_value(node, "colour")
            }
    )

@convertor("light")
def light_convertor(data, node):
    if node.parent.tag == "node":
        return

    if node.parent.tag == "scene":
        entity = data.get_entity("__lights__")
        if entity is None:
            entity = {
                        "id": "__lights__",
                        "render":
                        {
                            "root": 
                            {
                                "type": "static",
                                "lights": []
                            }
                        }
                    }
            data.add_entity(entity)

        entity["render"]["root"]["lights"].append(get_light_object(node))

@convertor("node")
def node_convertor(data, node):
    if node.parent.tag == "node":
        return

    def create_node(n):
        node_value = {
                "name": node.attrib["name"],
                "position": get_vector3(n),
                "rotation": get_quaternion(n, "rotation"),
                "scale": get_vector3(n, "scale")
        }

        child = n.find("entity")
        if child is not None:
            node_value["model"] = create_entity(child)

        def process_subnode(subnode):
            value = create_node(subnode)
            if "children" not in node_value:
                node_value["children"] = []
            node_value["children"].append(value)

        for child in n.iterfind("node"):
            process_subnode(child)

        for child in n.iterfind("light"):
            if "lights" not in node_value:
                node_value["lights"] = []
            node_value["lights"].append(get_light_object(child));
        return node_value

    root_node = create_node(node)
    entity = {
            "id": node.attrib["name"],
            "render": 
            {
                "root": root_node,
                "type": "static"
            }
        }

    data.add_entity(entity)


def create_entity(node):
    res = {
            "name": node.attrib["name"],
            "mesh": node.attrib["meshFile"],
            "castShadows": bool(node.attrib.get("castShadows", False))
    }
    for child in node.iterfind("subentity"):
        if "subEntities" not in res:
            res["subEntities"] = []
        res["subEntities"].insert(int(child.attrib["index"]), child.attrib["materialName"])

    return res

# --------------------------------------------------------------------------------
# Types shorthands
# --------------------------------------------------------------------------------

def get_quaternion(node, name, default="0,0,0,0"):
    node = node.find(name)
    if node is None:
        return default

    return "%(qw)s,%(qx)s,%(qy)s,%(qz)s" % node.attrib

def get_vector3(node, name="position", default="0,0,0"):
    node = node.find(name)
    if node is None:
        return default

    return "%(x)s,%(y)s,%(z)s" % node.attrib

def get_colour_value(node, name=None, default="0x00000000"):
    node = node.find(name) if name is not None else node
    if node is None:
        return default
    def get_hex(value):
        return int(float(value) * 0xFF)
    log(node.attrib)
    a = node.attrib.get("a", 0.0)
    r = node.attrib.get("r", 0.0)
    g = node.attrib.get("g", 0.0)
    b = node.attrib.get("b", 0.0)
    return "0x%x" % (get_hex(a) << 24 | get_hex(r) << 16 | get_hex(g) << 8 | get_hex(b));

# --------------------------------------------------------------------------------

if __name__ == "__main__":
    arg_parser = create_arg_parser()
    options = arg_parser.parse_args()
    dot_scene_parser = DotSceneParser()
    logging_enabled = options.verbose
    if options.verbose:
        log(open(options.input_file, "r").read())

    data = dot_scene_parser.decode(options.input_file)
    kwargs = {}
    res = ""
    if options.pretty:
        kwargs["indent"] = 2
    if options.dump_bson:
        import bson
        res = bson.dumps(data.output)
    else:
        res = json.dumps(data.output, **kwargs)
        log(res)

    f = open(options.output_file, "w")
    f.write(res)
    f.close()



