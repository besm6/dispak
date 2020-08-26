#
# Add dispak directory to the include path.
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../dispak
)

#
# Build 'disbesm6' binary.
#
add_executable(disbesm6
    disbesm6.c
    ../dispak/encoding.c
)
target_compile_options(disbesm6 PRIVATE -Wall -g -O2)
install(TARGETS disbesm6 DESTINATION bin)
