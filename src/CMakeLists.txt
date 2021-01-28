# Make an automatic library - will be static or dynamic based on user setting
add_library(mod router.cpp)

# Set up dependencies including directories, compile features, and external libraries
target_include_directories(mod PUBLIC ../include)

# All users of this library will need at least C++17
target_compile_features(mod PUBLIC cxx_std_17)

# External libraries
target_link_libraries(mod PUBLIC fmt::fmt)
