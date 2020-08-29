#!/usr/bin/env python3
#
# Run a test in current directory.
#
import sys, os

#--------------------------------------------------------------
# Parse the command line arguments:
#   arg.verbosity  - level of verbosity
#   arg.help       - usage info requested
#
import argparse
parser = argparse.ArgumentParser(add_help=False)
parser.add_argument("-v", "--verbosity", action="count", default=0)
parser.add_argument("-h", "--help", action="count", default=0)
arg = parser.parse_args()
if arg.help:
    print("Usage:")
    print("    run-test.py [options]")
    print("");
    print("Run test in current directory.");
    print("");
    print("Options:");
    print("    -v, --verbosity    Increase verbosity")
    print("    -h, --help         Show this message and exit")
    sys.exit(0)

#--------------------------------------------------------------
# Find application binary.
#
def get_app_path(app_name, relative_path):

    # Find application directory based on a path of this script.
    if getattr(sys, 'frozen', False):
        path = os.path.dirname(sys.executable)
    else:
        try:
            path = os.path.dirname(os.path.realpath(__file__))
        except NameError:
            return app_name

    # Try relative path.
    path = os.path.normpath(path + "/" + relative_path)
    if os.path.isfile(path):
        return path

    # No executable in the build directory: search the PATH.
    return app_name

#--------------------------------------------------------------
# Get options from file.
#
def get_options(file_name):
    if not os.path.isfile(file_name):
        print(f"FATAL: Cannot find {file_name} in current directory")
        sys.exit(-1)
    with open(file_name, 'r') as file:
        return ' '.join(file.readlines()).split()

#--------------------------------------------------------------
# Run application with given command line parameters.
#
def run_test(app, options, output_name):
    if arg.verbosity:
        print("Run:", app, options)

    import subprocess
    with open(output_name, 'w') as file:
        status = subprocess.call([app] + options, shell=False, stdout=file)

    if arg.verbosity:
        print("Status:", status)

    if status != 0:
        # The program terminated with error status.
        print(f"FATAL: {app} failed with status {status}")
        sys.exit(-1)

#--------------------------------------------------------------
# Compare the output with expected contents.
#
def compare_output(output_name, expected_name):
    if not os.path.isfile(expected_name):
        print(f"FATAL: Cannot find {expected_name} in current directory")
        print(f"FATAL: Please rename {output_name} to {expected_name}")
        sys.exit(-1)

    # Read file contents.
    with open(output_name, 'r') as file:
        output = file.readlines()
    with open(expected_name, 'r') as file:
        expected = file.readlines()

    import difflib
    return difflib.unified_diff(expected, output, n=0)

#--------------------------------------------------------------
# Match line against pattern.
# A symbol # in pattern matches any symbol in the line.
#
def match_line(line, pattern):
    for c, p in zip(line, pattern):
        #print("---", c, p)
        if p != '#' and c != p:
            return False
    return True

#--------------------------------------------------------------
# Compare the output with expected contents.
#
def process_wildcards(diff):
    # Convert the iterator into list.
    lines = list(diff)

    # Remove header.
    if len(lines) < 2 or \
       lines[0] != "--- \n" or \
       lines[1] != "+++ \n":
        return lines
    lines = lines[2:]

    # Process groups of three lines and match wildcards.
    while len(lines) >= 3:
        if lines[0][0] == '@' and \
           lines[1][0] == '-' and \
           lines[2][0] == '+' and \
           match_line(lines[2][1:], lines[1][1:]):
            # Match: remove these three lines.
            lines = lines[3:]
        else:
            # No match, return the rest.
            return lines

    return lines

#--------------------------------------------------------------
#
application_path = get_app_path("dispak", "../build/dispak/dispak")
if arg.verbosity:
    print("Application:", application_path)

options = get_options("options.txt")
if arg.verbosity:
    print("Options:", options)

run_test(application_path, options, "output.run")

diff = compare_output("output.run", "output.txt")

diff = process_wildcards(diff)

if len(diff) > 0:
    sys.stdout.writelines(diff)
    print("FATAL: Output does not match the sample")
    sys.exit(-1)
