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
target_compile_options(dispak PRIVATE -Wall -g -O3 -ffast-math -fomit-frame-pointer)
install(TARGETS dispak DESTINATION bin)
