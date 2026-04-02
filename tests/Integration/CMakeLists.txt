cmake_minimum_required(VERSION 3.22)
project(soar_integration_test LANGUAGES CXX)

# Locate the installed Soar package via its CMake config file.
# Pass -Dsoar_DIR=<path> or set CMAKE_PREFIX_PATH to the install prefix
# (e.g. the Conan generators directory) if Soar is not on the default search path.
find_package(soar REQUIRED)

add_executable(integration integration.cpp)
target_link_libraries(integration PRIVATE soar::soar_lib)
target_compile_features(integration PRIVATE cxx_std_17)

enable_testing()
add_test(NAME soar_find_package_integration COMMAND integration)
