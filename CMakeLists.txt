# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  mod-abm-2.0
  VERSION 0.1
  DESCRIPTION "An agent-based modeling platform for mobility-on-demand simulations"
  LANGUAGES CXX)

# Define the path to cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find all required libraries including the OSRM-backend library(LibOSRM)
find_package(LibOSRM REQUIRED)
find_package(Boost 1.52.0 COMPONENTS filesystem system thread iostreams chrono date_time regex REQUIRED)

# The libraries
add_library(model src/router.cpp)

# The executable
add_executable(main src/main.cpp)
target_link_libraries(main model ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})

# More linking for LibOSRM
link_directories(${LibOSRM_LIBRARY_DIRS})
include_directories(SYSTEM ${LibOSRM_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LibOSRM_CXXFLAGS}")

# Third-party libraries
include(FetchContent)
include(ExternalProject)

# Add fmt::fmt
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 5.3.0)
FetchContent_MakeAvailable(fmtlib)

# Add Google test suite
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.8.0
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# Set up tests
enable_testing()
include(GoogleTest)

add_executable(model_test test/types_test.cpp test/router_test.cpp)
target_link_libraries(model_test gtest gmock gtest_main model ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})
gtest_discover_tests(model_test
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
set_target_properties(model_test PROPERTIES FOLDER tests)