#!/usr/bin/env python3
#
# Show a list of available tests.
#
import sys, os

#--------------------------------------------------------------
# Parse the command line arguments:
#   arg.update    - update file CMakeLists.txt with a new list of tests
#   arg.verbosity - level of verbosity
#   arg.help      - usage info requested
#
import argparse
parser = argparse.ArgumentParser(add_help=False)
parser.add_argument("-u", "--update", action="count", default=0)
parser.add_argument("-v", "--verbosity", action="count", default=0)
parser.add_argument("-h", "--help", action="count", default=0)
arg = parser.parse_args()
if arg.help:
    print("Usage:")
    print("    show_tests.py [options]")
    print("");
    print("Print a list of Dispak tests.");
    print("");
    print("Options:");
    print("    -u, --update       Update file CMakeLists.txt")
    print("    -v, --verbosity    Increase verbosity")
    print("    -h, --help         Show this message and exit")
    sys.exit(0)

#if arg.verbosity >= 1:
#    print("TODO")

#--------------------------------------------------------------
# Build a list of tests.
# Return a dictionary { test_name: options }.
#
def scan_tests():
    result = {}
    for dir in os.listdir("."):
        if os.path.isdir(dir):
            file_name = dir + "/options.txt"
            if os.path.isfile(file_name):
                with open(file_name, 'r') as file:
                    options = ' '.join(file.readlines())
                    options = ' '.join(options.split())
                    result[dir] = options
    return result

#--------------------------------------------------------------
# Print tests.
# For every test, show the options.
#
def print_tests(tests):
    print("Test                     Options")
    print("-------------------------------------------------")
    for dir in sorted(tests):
        options = tests[dir]
        print(f"{dir:24} {options}")

#--------------------------------------------------------------
# Update a list of tests in file .
#
def update_cmakelists(tests, file_name):
    count = 0
    with open(file_name, 'w') as file:
        for dir in sorted(tests):
            file.write(f"add_test(NAME {dir} COMMAND Python3::Interpreter ../run-test.py WORKING_DIRECTORY ${{CMAKE_SOURCE_DIR}}/tests/{dir})\n")
            count += 1
    print(f"\nFile {file_name} updated with {count} tests.")

#
# Main procedure.
#
tests = scan_tests()

print_tests(tests)

if arg.update > 0:
    update_cmakelists(tests, "CMakeLists.txt")
