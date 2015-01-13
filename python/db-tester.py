#!/usr/bin/python3
"""
Runs the test suite.
"""

import argparse
import os
import sys
from collections import defaultdict

def is_valid_test_path(path):
    parts = path.split('.')

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
        

def path_key(path):
    if not is_valid_test_path(path):
        return None

    parts = path.split('.')
    return parts[0]


if __name__ == '__main__':
    if len(sys.argv) is not 2:
        print("usage:", sys.argv[0], "path")
        sys.exit()

    path = sys.argv[1]

    paths = set([f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))])
    valid_paths = set([f for f in paths if is_valid_test_path(f)])
    invalid_paths = paths.difference(valid_paths)

    paths_dict = defaultdict(list)
    for p in paths:
        paths_dict[path_key(p)].append(p)

    print("paths:", paths)
    print("valid_paths:", valid_paths)
    print("invalid_paths:", invalid_paths)
    print("paths_dict:", paths_dict)

