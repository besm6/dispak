cmake_minimum_required(VERSION 2.8)
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

# Build version name as: tag.revcount-hash
# For example: v1.0.123-abcdef9
add_definitions("-DPACKAGE_VERSION=\"${GIT_TAG}.${GIT_REVCOUNT}-${GIT_HASH}\"")

#
# Build subdirectories.
#
add_subdirectory(dispak)
add_subdirectory(besmtool)
add_subdirectory(disbesm6)