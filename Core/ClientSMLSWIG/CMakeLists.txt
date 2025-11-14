# SWIG Language Bindings for Soar SML
cmake_minimum_required(VERSION 3.23)

# Options for different language bindings
option(SWIG_PYTHON "Build Python SWIG bindings" OFF)
option(SWIG_JAVA "Build Java SWIG bindings" OFF)

# Find SWIG (required for all bindings)
if(SWIG_PYTHON OR SWIG_JAVA)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})

    # Common SWIG flags
    set(CMAKE_SWIG_FLAGS -c++)

    if(NOT SVS)
        list(APPEND CMAKE_SWIG_FLAGS -DNO_SVS)
    endif()

    # Common include directories for SWIG
    set(SWIG_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../ClientSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../ConnectionSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../ElementXML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../KernelSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../SoarKernel/src/shared
    )
endif()

# Python bindings
if(SWIG_PYTHON)
    message(STATUS "Soar SWIG: Building Python bindings")

    # Find Python
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

    # Configure SWIG for Python
    set_property(SOURCE Python/Python_sml_ClientInterface.i PROPERTY CPLUSPLUS ON)

    # Set Python-specific SWIG flags using target properties
    set(PYTHON_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -python)

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND PYTHON_SWIG_FLAGS -I${dir})
    endforeach()

    # Set output directory
    set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/python")

    # Create the SWIG module
    swig_add_library(soar_sml_python
        TYPE SHARED
        LANGUAGE python
        SOURCES Python/Python_sml_ClientInterface.i
    )

    # Set Python-specific SWIG flags for this target
    set_property(TARGET soar_sml_python PROPERTY SWIG_COMPILE_OPTIONS ${PYTHON_SWIG_FLAGS})

    # Set properties for the SWIG module
    set_target_properties(soar_sml_python PROPERTIES
        OUTPUT_NAME "Python_sml_ClientInterface"
        SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )

    # Link against soar library and Python
    target_link_libraries(soar_sml_python soar_lib Python3::Python)

    # Include directories
    target_include_directories(soar_sml_python PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${SWIG_INCLUDE_DIRS}
    )

    # Platform-specific settings
    if(UNIX AND NOT APPLE)
        target_compile_options(soar_sml_python PRIVATE -Wno-unused -fno-strict-aliasing)
    endif()

    if(APPLE)
        set_target_properties(soar_sml_python PROPERTIES
            LINK_FLAGS "-bundle -flat_namespace -undefined suppress"
            SUFFIX ".so"
        )
    endif()

    # Install Python files
    install(TARGETS soar_sml_python
        LIBRARY DESTINATION python
    )

    install(FILES "${CMAKE_SWIG_OUTDIR}/Python_sml_ClientInterface.py"
        DESTINATION python
    )

    install(FILES "Python/TestPythonSML.py"
        DESTINATION python
    )
endif()

# Java bindings
if(SWIG_JAVA)
    message(STATUS "Soar SWIG: Building Java bindings")

    # Find Java and JNI
    find_package(Java REQUIRED COMPONENTS Development Runtime)
    find_package(JNI REQUIRED)

    # Configure SWIG for Java
    set_property(SOURCE Java/Java_sml_ClientInterface.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE Java/Java_sml_ClientInterface.i PROPERTY SWIG_MODULE_NAME sml)

    # Set Java-specific SWIG flags using target properties
    set(JAVA_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -java)

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND JAVA_SWIG_FLAGS -I${dir})
    endforeach()

    # Add Java directory to SWIG flags
    list(APPEND JAVA_SWIG_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/Java)

    # Set output directory for Java files
    set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/java")

    # Copy JavaCallbackByHand.h to the build directory
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Java/JavaCallbackByHand.h"
        "${CMAKE_SWIG_OUTDIR}/JavaCallbackByHand.h"
        COPYONLY
    )

    # Create the SWIG module
    swig_add_library(soar_sml_java
        TYPE SHARED
        LANGUAGE java
        SOURCES Java/Java_sml_ClientInterface.i
    )

    # Set Java-specific SWIG flags for this target
    set_property(TARGET soar_sml_java PROPERTY SWIG_COMPILE_OPTIONS ${JAVA_SWIG_FLAGS})

    # Set properties for the SWIG module
    set_target_properties(soar_sml_java PROPERTIES
        OUTPUT_NAME "Java_sml_ClientInterface"
    )

    # Link against soar library and JNI
    target_link_libraries(soar_sml_java soar_lib ${JNI_LIBRARIES})

    # Include directories
    target_include_directories(soar_sml_java PRIVATE
        ${JNI_INCLUDE_DIRS}
        ${SWIG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Java
        ${CMAKE_SWIG_OUTDIR}
    )

    # Platform-specific settings
    if(UNIX AND NOT APPLE)
        target_compile_options(soar_sml_java PRIVATE -Wno-unused -fno-strict-aliasing)
    endif()

    # Create a script to compile Java files and create JAR
    set(COMPILE_JAVA_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/compile_java.sh")
    file(WRITE "${COMPILE_JAVA_SCRIPT}"
        "#!/bin/bash
set -e
cd \"${CMAKE_SWIG_OUTDIR}\"
mkdir -p classes
${Java_JAVAC_EXECUTABLE} -d classes *.java
${Java_JAR_EXECUTABLE} cf sml.jar -C classes .
echo \"Java compilation and JAR creation completed\"
")

    add_custom_command(
        TARGET soar_sml_java POST_BUILD
        COMMAND chmod +x "${COMPILE_JAVA_SCRIPT}"
        COMMAND "${COMPILE_JAVA_SCRIPT}"
        COMMENT "Compiling Java files and creating JAR"
        VERBATIM
    )

    # Install Java files
    install(TARGETS soar_sml_java
        LIBRARY DESTINATION java
    )

    # Install generated Java source files
    install(DIRECTORY "${CMAKE_SWIG_OUTDIR}/"
        DESTINATION java
        FILES_MATCHING PATTERN "*.java"
    )

    # Install JAR file
    install(FILES "${CMAKE_SWIG_OUTDIR}/sml.jar"
        DESTINATION java
    )
endif()
