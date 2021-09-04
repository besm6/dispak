cmake_minimum_required(VERSION 2.9)
project(dispak)

# Get the tag
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the revision count
execute_process(
    COMMAND git rev-list HEAD --count
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVCOUNT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#
# Build subdirectories.
#
add_subdirectory(dispak)
add_subdirectory(besmtool)
add_subdirectory(disbesm6)

#
# Add 'make test' target.
#
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    enable_testing()
    add_subdirectory(tests)
endif()
