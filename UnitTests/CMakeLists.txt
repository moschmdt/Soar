cmake_minimum_required(VERSION 3.22)
project(test_soar
  LANGUAGES CXX)

file(GLOB_RECURSE SOURCES
    TestHarness/*.cpp
    SoarHelpers/*.cpp
    SoarUnitTests/*.cpp
    )

file(GLOB_RECURSE HEADERS
    TestHarness/*.hpp
    SoarHelpers/*.hpp
    SoarUnitTests/*.hpp
    )

add_library(test_lib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(test_lib soar)
set_target_properties(test_lib
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}")

target_include_directories(test_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTest>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/TestHarness>
)

add_executable(${PROJECT_NAME} testMain.cpp)
target_link_libraries(${PROJECT_NAME} test_lib)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTest>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/TestHarness>
)

install(TARGETS ${PROJECT_NAME} test_lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY SoarTestAgents DESTINATION ${CMAKE_INSTALL_PREFIX}/SoarUnitTests/)

# TESTS FROM CI:
# run: ./UnitTests -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton
add_test(NAME UNIT_TEST COMMAND ${PROJECT_NAME} -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton)
