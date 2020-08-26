#
# Add dispak directory to the include path.
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../dispak
    ${CMAKE_CURRENT_BINARY_DIR}/../dispak
)

#
# Build 'besmtool' binary.
#
add_executable(besmtool
    besmtool.c
    write.c
    dump.c
    list.c
    erase.c
    passports.c
    ../dispak/disk.c
    ../dispak/encoding.c
)
target_compile_options(besmtool PRIVATE -Wall -g -O2)
install(TARGETS besmtool DESTINATION bin)
