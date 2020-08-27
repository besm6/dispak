# Simulator of BESM-6 computer at user level

The simulator takes a pack of punch cards on input and runs them
as a task for DISPAK operating system. Calls to the OS (extracodes)
are simulated as needed. Tasks can run either in batch mode, or in
interactive mode.

A format of the input is described in [Input-Format](Input-Format).

The simulator consists of three components:
 - dispak - the simulator
 - besmtool - a utility for dealing with images of BESM-6 disks and tapes
 - disbesm6 - a disassembler of BESM-6 binaries

# Build

    mkdir build
    cd build
    cmake ..
    make
    make install

# Testing
To run all available tests:

    cd build
    make test

Expected output:

    $ make test
    Running tests...
    Test project /Users/vak/Project/Besm-6/dispak/build
        Start 1: algol-besm6
    1/3 Test #1: algol-besm6 ......................   Passed    0.05 sec
        Start 2: algol-gdr
    2/3 Test #2: algol-gdr ........................   Passed    0.06 sec
        Start 3: fortran-dubna
    3/3 Test #3: fortran-dubna ....................   Passed    0.09 sec

    100% tests passed, 0 tests failed out of 3

    Total Test time (real) =   0.20 sec

To see, why a particular test fails, use:

    cd build
    ctest -V -R test-name

To create a new test:
  * Create a directory for it, say tests/mytest
  * Put here a file with input for dispak, for example 'mytest.b6'.
  * Create a file tests/mytest/options.txt with command line parameters for dispak. The file should contains at least a filename of the input file, i.e. mytest.b6.
  * Create a file with expected output 'output.txt'. It can be obtained by manually running dispak and redirecting the output to a file. For example: "dispak mytest.b6 > output.txt".
  * Add a line to the file tests/CMakeLists.txt for your test. Like this:

    add_test(NAME mytest COMMAND Python3::Interpreter ../run-test.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/mytest)
