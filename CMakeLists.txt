cmake_minimum_required(VERSION 3.22)
project(Soar VERSION 9.6.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_CPPLINT "cpplint")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include(GNUInstallDirs)

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
# include_directories(
# )
add_compile_definitions(NO_SVS)

# Add executable/library target
add_library(Soar SHARED ${SOURCES})
target_include_directories(Soar PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/CLI/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ClientSML/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ConnectionSML/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ElementXML/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/KernelSML/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/sqlite>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/debug_code>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/decision_process>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/episodic_memory>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/explanation_based_chunking>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/explanation_memory>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/interface>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/output_manager>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/parsing>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/reinforcement_learning>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/semantic_memory>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/shared>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/soar_representation>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/visualizer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/shared>
)
set_target_properties(Soar
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}")

# Specify installation directory for the library and headers
# install(TARGETS Soar
# LIBRARY DESTINATION lib
# )

# This only sets up the install structure, but no installation happens yet.
install(TARGETS Soar
    EXPORT SoarTarget
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT SoarTarget
    FILE SoarTargets.cmake
    NAMESPACE Soar::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Soar
)
