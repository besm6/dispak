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
