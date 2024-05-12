cmake_minimum_required(VERSION 3.22)
project(Soar VERSION 9.6.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_CPPLINT "cpplint")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Add your source files
set(SOURCES
    Core/ClientSML/ClientSML.cxx
    Core/ConnectionSML/ConnectionSML.cxx
    Core/ElementXML/ElementXML.cxx
    Core/KernelSML/KernelSML.cxx
    Core/SoarKernel/SoarKernel.cxx
    Core/SoarKernel/sqlite/sqlite3.c
    Core/shared/shared.cxx
    # Core/CLI/CommandLineInterface.cxx
)

file(GLOB_RECURSE HEADERS
    "Core/CLI/src/*.h"
    "Core/ClientSML/src/*.h"
    "Core/ConnectionSML/src/*.h"
    "Core/ElementXML/src/*.h"
    "Core/KernelSML/src/*.h"
    "Core/SoarKernel/sqlite/*.h"
    "Core/SoarKernel/src/debug_code/*.h"
    "Core/SoarKernel/src/decision_process/*.h"
    "Core/SoarKernel/src/episodic_memory/*.h"
    "Core/SoarKernel/src/explanation_based_chunking/*.h"
    "Core/SoarKernel/src/explanation_memory/*.h"
    "Core/SoarKernel/src/interface/*.h"
    "Core/SoarKernel/src/output_manager/*.h"
    "Core/SoarKernel/src/parsing/*.h"
    "Core/SoarKernel/src/reinforcement_learning/*.h"
    "Core/SoarKernel/src/semantic_memory/*.h"
    "Core/SoarKernel/src/shared/*.h"
    "Core/SoarKernel/src/soar_representation/*.h"
    "Core/SoarKernel/src/visualizer/*.h"
    "Core/shared/*.h"
    )


# Set include directories
include_directories(
    Core/CLI/src
    Core/ClientSML/src
    Core/ConnectionSML/src
    Core/ElementXML/src
    Core/KernelSML/src
    Core/SoarKernel/sqlite
    Core/SoarKernel/src/debug_code
    Core/SoarKernel/src/decision_process
    Core/SoarKernel/src/episodic_memory
    Core/SoarKernel/src/explanation_based_chunking
    Core/SoarKernel/src/explanation_memory
    Core/SoarKernel/src/interface
    Core/SoarKernel/src/output_manager
    Core/SoarKernel/src/parsing
    Core/SoarKernel/src/reinforcement_learning
    Core/SoarKernel/src/semantic_memory
    Core/SoarKernel/src/shared
    Core/SoarKernel/src/soar_representation
    Core/SoarKernel/src/visualizer
    Core/shared
)

add_compile_definitions(NO_SVS)

# Add executable/library target
add_library(Soar SHARED ${SOURCES})
target_include_directories(Soar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(Soar
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}")

# Specify installation directory for the library and headers
install(TARGETS Soar
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/Soar
)
