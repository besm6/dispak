# How to add a new test

## 1. Create directory

Create a directory for the new test, say 'tests/mytest'.

## 2. Input file

Put here a file with input for dispak, for example 'tests/mytest/mytest.b6'.

## 3. Options

Create a file 'tests/mytest/options.txt' with command line parameters for dispak.
The file should contains at least a filename of the input file, for example:

    mytest.b6

Here is a list of available options, from 'dispak --help':

    -x, --native           use native extracode E64
    -t, --trace            trace all extracodes
    --trace-e64            trace extracode 064
    -s, --stats            show statistics for machine instructions
    -p, --output-enable    display printing output (default for batch tasks)
    -q, --output-disable   no printing output (default for TELE tasks)
    -l, --output-latin     use Latin letters for output
    --output-cyrillic      use Cyrillic letters for output (default)
    --punch-binary         punch in binary format (default dots and holes)
    --input-encoding=code  set encoding for input files: utf8 koi8 cp1251 cp866
    --no-insn-check        all words but at addr 0 are treated as insns

## 4. Expected output

Create a file with expected output 'output.txt'. It can be obtained by
manually running dispak and redirecting the output to a file.
For example:

    cd tests/mytest
    dispak mytest.b6 > output.txt

## 5. Update the list

File tests/CMakeLists.txt contains a list of all tests.
You need to update it to include your test.
This can be done by script:

    cd tests
    ./show-tests.py -u
