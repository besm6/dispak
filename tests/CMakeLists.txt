add_test(NAME algol-besm6 COMMAND Python3::Interpreter ../run-test.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/algol-besm6)
add_test(NAME algol-gdr COMMAND Python3::Interpreter ../run-test.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/algol-gdr)
add_test(NAME fortran-dubna COMMAND Python3::Interpreter ../run-test.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/fortran-dubna)
