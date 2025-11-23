# Qt Soar Debugger
# A native C++ Qt application for debugging Soar agents

cmake_minimum_required(VERSION 3.23)

# Only build if Qt is available and explicitly requested
option(BUILD_QT_DEBUGGER "Build Qt-based Soar debugger" ON)

if(NOT BUILD_QT_DEBUGGER)
    message(STATUS "Qt debugger disabled")
    return()
endif()

# Find Qt6 - add Wayland support on Linux
if(UNIX AND NOT APPLE)
    find_package(Qt6 COMPONENTS Core Widgets WaylandClient QUIET)
else()
    find_package(Qt6 COMPONENTS Core Widgets QUIET)
endif()

if(NOT Qt6_FOUND)
    message(STATUS "Qt6 not found, trying Qt5")
    find_package(Qt5 COMPONENTS Core Widgets QUIET)

    if(NOT Qt5_FOUND)
        message(STATUS "Neither Qt6 nor Qt5 found, skipping Qt debugger")
        return()
    endif()

    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5 for debugger")
else()
    set(QT_VERSION_MAJOR 6)

    if(UNIX AND NOT APPLE)
        message(STATUS "Using Qt6 for debugger with Wayland support (Linux)")
    else()
        message(STATUS "Using Qt6 for debugger")
    endif()
endif()

# Set up Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AgentWindow.cpp
    src/SoarDebugger.cpp
    src/OutputWindow.cpp
    src/CommandWindow.cpp
    src/TracesWindow.cpp
    src/PreferencesWindow.cpp
    src/WorkingMemoryTree.cpp
    src/ProductionsWindow.cpp
    src/SoarAgent.cpp
)

set(HEADERS
    src/MainWindow.h
    src/AgentWindow.h
    src/SoarDebugger.h
    src/OutputWindow.h
    src/CommandWindow.h
    src/TracesWindow.h
    src/PreferencesWindow.h
    src/WorkingMemoryTree.h
    src/ProductionsWindow.h
    src/SoarAgent.h
)

# UI files
set(UI_FILES
    ui/MainWindow.ui
    ui/AgentWindow.ui
    ui/OutputWindow.ui
    ui/CommandWindow.ui
    ui/TracesWindow.ui
    ui/PreferencesWindow.ui
    ui/WorkingMemoryTree.ui
    ui/ProductionsWindow.ui
)

# Create the executable
add_executable(soar_qt_debugger ${SOURCES} ${HEADERS} ${UI_FILES})

# Link Qt libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(soar_qt_debugger Qt6::Core Qt6::Widgets)

    # Add Wayland support on Linux
    if(UNIX AND NOT APPLE)
        target_link_libraries(soar_qt_debugger Qt6::WaylandClient)

        # Force Wayland platform on Linux
        target_compile_definitions(soar_qt_debugger PRIVATE QT_QPA_PLATFORM=wayland)
    endif()
endif()

# Link Soar libraries
target_link_libraries(soar_qt_debugger soar_lib)

# Include directories
target_include_directories(soar_qt_debugger PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR} # For generated files
    ${CMAKE_SOURCE_DIR}/Core/ClientSML/src # SML source headers
)

# Set properties
set_target_properties(soar_qt_debugger PROPERTIES
    OUTPUT_NAME "SoarDebugger"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Install
install(TARGETS soar_qt_debugger
    RUNTIME DESTINATION bin
    COMPONENT qtdebugger
)

# Install Qt runtime if needed (Windows)
if(WIN32 AND QT_VERSION_MAJOR EQUAL 6)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_PREFIX_PATH}/bin)

    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/bin/SoarDebugger.exe)")
    endif()
elseif(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt5_PREFIX_PATH}/bin)

    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/bin/SoarDebugger.exe)")
    endif()
endif()

message(STATUS "Qt Soar Debugger will be built")