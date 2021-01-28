# macro
macro(package_add_test TESTNAME FILES LIBRARIES)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${FILES})

    # link the Google test infrastructure, mocking library, and a default main fuction to the test executable. Also include user-defined libraries.
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})

    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )

    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(testlib types_test.cpp mod)