#
# Create config.h.
#
configure_file(config.h.in config.h @ONLY)

#
# Include both source and build directories.
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Check for system include files.
#
include(CheckIncludeFile)
check_include_file("string.h" HAVE_STRING_H)

#
# Need Bison parser.
#
find_package(BISON REQUIRED)
bison_target(debug debug.y "${CMAKE_CURRENT_BINARY_DIR}/debug.c")

#
# Build 'dispak' binary.
#
add_executable(dispak
    dispak.c
    cu.c
    optab.c
    arith.c
    input.c
    extra.c
    disk.c
    errtxt.c
    vsinput.c
    dpout.c
    encoding.c
    getopt.c
    "${CMAKE_CURRENT_BINARY_DIR}/debug.c"
)
target_link_libraries(dispak PUBLIC m)
target_compile_options(dispak PRIVATE
    -Wall -g -O3 -ffast-math -fomit-frame-pointer -DHAVE_STRING_H=${HAVE_STRING_H}
)
install(TARGETS dispak DESTINATION bin)

#
# 'make 2099' builds image of disk 2099.
#
add_custom_target(2099 ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/2099)

#
# Put disk images into this directory.
#
set(DISKDIR $ENV{HOME}/.besm6)

#
# List of disks to downloads.
#
set(DISKS 2048 2053 2148 2153 2113 2248 4001 4002 2063 2086 2099)

#
# Build disk 2099.
#
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/2099
    DEPENDS dispak make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating 2099"
    COMMAND rm -f 2099
         && touch 2099
         && ${CMAKE_CURRENT_SOURCE_DIR}/install_disks.sh . 2053 2048 2248
         && ln -s -f ${DISKDIR}/2053 2053
         && ln -s -f ${DISKDIR}/2048 2048
         && ln -s -f ${DISKDIR}/2248 2248
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/macros.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/disp99.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/e64.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/ekdisp.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/sostav.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/spe66.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/make2099.b6
         && rm -f 2053 2048 2248
)

#
# On 'make install', copy disk images to ~/.besm6/ directory.
#
install(CODE "execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install_disks.sh ${CMAKE_CURRENT_BINARY_DIR} ${DISKS})")
