# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  mod-abm-2.0
  VERSION 0.1
  DESCRIPTION "An agent-based modeling platform for mobility-on-demand simulations"
  LANGUAGES CXX)

########################################################################
# Find External Packages
########################################################################

# Define the path to cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find all required libraries including the OSRM-backend library(LibOSRM)
find_package(LibOSRM REQUIRED)
find_package(Boost 1.52.0 COMPONENTS filesystem system thread iostreams chrono date_time regex REQUIRED)

########################################################################
# Define Libraries and Executable
########################################################################

# The libraries
add_library(mod-abm-lib src/config.cpp src/demand_generator.cpp src/router.cpp src/vehicle.cpp )
target_link_libraries(mod-abm-lib yaml-cpp fmt::fmt ${Python2_INCLUDE_DIRS})
target_compile_features(mod-abm-lib PRIVATE cxx_std_17)

# The executable
add_executable(main src/main.cpp)
target_link_libraries(main mod-abm-lib yaml-cpp fmt::fmt ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})
target_compile_features(main PRIVATE cxx_std_17)

# More linking for LibOSRM
link_directories(${LibOSRM_LIBRARY_DIRS})
include_directories(SYSTEM ${LibOSRM_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LibOSRM_CXXFLAGS}")

########################################################################
# Fetch Other Third-Party Libraries
########################################################################

# Third-party libraries
include(FetchContent)
include(ExternalProject)

# Add fmt::fmt
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 5.3.0)
FetchContent_MakeAvailable(fmtlib)

# Add YAML
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.6.3)
FetchContent_MakeAvailable(yaml-cpp)

# Add Google test suite
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.8.0
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  message(STATUS ${googletest_SOURCE_DIR})
  message(STATUS ${googletest_BINARY_DIR})
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# Add Google benchmark suite
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.5.2
)
FetchContent_GetProperties(googlebenchmark)
if(NOT benchmark_POPULATED)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "")
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
  FetchContent_MakeAvailable(googlebenchmark)
endif()

########################################################################
# Tests
########################################################################

# Enable testing
enable_testing()
include(GoogleTest)

# Add executable for all test cases
add_executable(run_all_tests test/types_test.cpp test/router_test.cpp test/vehicle_test.cpp)
target_link_libraries(run_all_tests gtest gmock gtest_main mod-abm-lib ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})
gtest_discover_tests(run_all_tests
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
set_target_properties(run_all_tests PROPERTIES FOLDER tests)

########################################################################
# Benchmarks
########################################################################

# Benchmarks
add_executable(router_benchmark benchmark/router_benchmark.cpp)
target_link_libraries(router_benchmark benchmark::benchmark mod-abm-lib ${LibOSRM_LIBRARIES} ${LibOSRM_DEPENDENT_LIBRARIES})