# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.
set(HEADER_LIST "${PROJECT_SOURCE_DIR}/include/mod/lib.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(mod lib.cpp ${HEADER_LIST})

# Set up dependencies including directories, compile features, and external libraries
target_include_directories(mod PUBLIC ../include)

# All users of this library will need at least C++17
target_compile_features(mod PUBLIC cxx_std_17)

# External libraries
target_link_libraries(mod PUBLIC fmt::fmt)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
