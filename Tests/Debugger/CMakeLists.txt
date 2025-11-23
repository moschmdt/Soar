cmake_minimum_required(VERSION 3.23)
project(SpawnDebuggerTest
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Soar library - Conan will provide all transitive dependencies
find_package(soar REQUIRED CONFIG)

# Create the test executable
add_executable(spawn_debugger_test SpawnDebugger.cpp)

# Link to Soar - dependencies (SQLite3, Boost) are automatically propagated by Conan
target_link_libraries(spawn_debugger_test PRIVATE soar::soar_lib)

# Optional: Add install target
install(TARGETS spawn_debugger_test
    RUNTIME DESTINATION bin
)

# Print configuration info
message(STATUS "")
message(STATUS "=== SpawnDebugger Test Configuration ===")
message(STATUS "Using Conan package: soar/9.6.4")
message(STATUS "All dependencies (SQLite3, Boost) are transitive")
message(STATUS "========================================")
message(STATUS "")
