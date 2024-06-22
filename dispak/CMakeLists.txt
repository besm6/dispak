#
# Download disk images from this site.
#
set(ARCHIVE "http://www.besm6.org/download/disks")

#
# Put disk images into this directory.
#
set(DISKDIR ${CMAKE_INSTALL_PREFIX}/share/besm6)

#
# List of disks to download.
#
set(DISKS 2048 2053 2148 2153 2113 2248 4001 2063 2086 2099)

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
# On 'make install', copy disk images to /usr/local/share/besm6/ directory.
#
install(CODE "execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install_disks.sh ${DISKDIR} ${CMAKE_CURRENT_BINARY_DIR} ${DISKS})")

#
# Build disk 2099.
#
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/2099
    DEPENDS dispak make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6
        ${CMAKE_CURRENT_BINARY_DIR}/2053
        ${CMAKE_CURRENT_BINARY_DIR}/2048
        ${CMAKE_CURRENT_BINARY_DIR}/2248
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating 2099"
    COMMAND rm -f 2099
         && touch 2099
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/macros.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/disp99.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/e64.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/ekdisp.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/sostav.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/spe66.b6
         && ./dispak -q --bootstrap --path=. --input-encoding=utf8 ${CMAKE_CURRENT_SOURCE_DIR}/make2099.b6
)

#
# Build disks 2053, 2048, 2248.
#
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/2053
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating 2053"
    COMMAND if [ -e "${DISKDIR}/2053" ]\;
        then cp -a -L "${DISKDIR}/2053" .\;
        else wget -nv -O 2053 ${ARCHIVE}/sbor2053.bin\; fi
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/2048
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating 2048"
    COMMAND if [ -e "${DISKDIR}/2048" ]\;
        then cp -a -L "${DISKDIR}/2048" .\;
        else wget -nv -O 2048 ${ARCHIVE}/sbor2048.bin\; fi
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/2248
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating 2248"
    COMMAND if [ -e "${DISKDIR}/2248" ]\;
        then cp -a -L "${DISKDIR}/2248" .\;
        else wget -nv -O 2248 ${ARCHIVE}/alt2048.bin\; fi
)
