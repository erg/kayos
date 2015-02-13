#!/usr/bin/env python3
"""
Runs the test suite.
"""

import argparse
import os
import sys
import json
import ntpath
from pprint import pprint
from collections import defaultdict

# http://stackoverflow.com/questions/8384737/python-extract-file-name-from-path-no-matter-what-the-os-path-format
def path_to_filename(path):
    head, tail = ntpath.split(path)
    return tail or ntpath.basename(head)

def is_valid_test_path(path):
    parts = path_to_filename(path).split('.')

    valid_parts_1 = ["input", "output"]
    valid_parts_2 = ["json","http","telnet","binary","pubsub"]

    if len(parts) != 3:
        print("error, path should be of the form test-name.{%s}.{%s}", ",".join(valid_parts_1), ",".join(valid_parts_2))
        print("  instead got path:", path)
        return False

    if not any(parts[1] in valid_parts_1 for p in parts):
        print("error, second part of path should be {input,output}")
        print("  instead got path:", parts[1])
        return False

    if not any(parts[2] in valid_parts_2 for p in parts):
        print("error, second part of path should be {json,http,telnet,binary,pubsub}")
        print("  instead got path:", parts[2])
        return False
    return True

def tokenize_path(path):
    parts = path_to_filename(path).split('.')
    return parts

def path_key(path):
    return tokenize_path(path)[0]

def is_input_path(path):
    return tokenize_path(path)[1] == "input"

def is_output_path(path):
    return tokenize_path(path)[1] == "output"

def path_method(path):
    return tokenize_path(path)[2]


def handle_json_input(path):
    print("loading json from path:", path)
    json = load_json_file(path)
    #print("json:",)
    #pprint(json)

def handle_http_input(path):
    pass

def handle_telnet_input(path):
    pass

def handle_binary_input(path):
    pass

def handle_pubsub_input(path):
    pass


def handle_input_path(path):
    parts = tokenize_path(path)
    method = parts[2]

    if method == "json":
        print("method: json")
        handle_json_input(path)
    elif method == "http":
        print("method: http")
        #handle_http_input(path)
    elif method == "telnet":
        print("method: telnet")
        #handle_telnet_input(path)
    elif method == "binary":
        print("method: binary")
        #handle_binray_input(path)
    elif method == "pubsub":
        print("method: pubsub")
        #handle_pubsub_input(path)
    else:
        print("unknown method:", method)
        sys.exit()


#def dump_db(path):
    # dump the db to


def invalid_path_error(path):
    print("invalid path:", path)
    sys.exit()

def get_file_contents(filename):
    with open(filename) as f:
        return f.read()

def load_json_file(path):
    #json_data = open('json_data')
    #data = json.load(json_data)
    #json_data.close()
    #return data
    with open(path) as f:
        return json.load(f)

if __name__ == '__main__':
    if len(sys.argv) is not 2:
        print("usage:", sys.argv[0], "path")
        sys.exit()

    directory = sys.argv[1]

    all_paths = set([f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))])
    valid_paths = set([f for f in all_paths if is_valid_test_path(f)])
    invalid_paths = all_paths.difference(valid_paths)

    if len(invalid_paths) > 0:
        print("invalid paths:", invalid_paths)
        sys.exit()

    paths_dict = defaultdict(list)
    for p in all_paths:
        paths_dict[path_key(p)].append(p)

    #print("paths:", paths)
    #print("valid_paths:", valid_paths)
    #print("invalid_paths:", invalid_paths)
    print("paths_dict:", paths_dict)

    num_tests = len(paths_dict)
    errors = list()

    for k, paths in paths_dict.items():
        expected = None
        actual = None
        input_paths = set([os.path.join(directory, path) for path in paths if is_input_path(path)])
        output_paths = [os.path.join(directory, path) for path in paths if is_output_path(path)]

        print("k", k)
        print("input paths", input_paths)
        print("output paths", output_paths)

        if len(output_paths) == 0:
            print("error: found no output file for key:", k)
            sys.exit()

        if len(output_paths) > 1:
            print("error: found more than one output file")
            sys.exit()

        print("loading output:", output_paths[0])
        expected = load_json_file(output_paths[0])
        #actual = dump_database(k)
        #pprint(output)

        for path in input_paths:
            handle_input_path(path)

        print()

    if len(errors) == 0:
        print("tests pass: %d/%d" % (num_tests, num_tests))
    else:
        print("tests failed: " + errors)
