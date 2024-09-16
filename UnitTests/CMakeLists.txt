cmake_minimum_required(VERSION 3.22)
project(test_soar
  LANGUAGES CXX)

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

include(CTest)

file(GLOB TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/*.cpp
    )

file(GLOB TEST_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/*.hpp
    )

add_executable(${PROJECT_NAME} testMain.cpp ${TEST_SOURCES} ${TEST_HEADERS})
target_link_libraries(${PROJECT_NAME} soar)

add_executable(test_external_lib TestExternalLibraryLib.cpp)
target_link_libraries(test_external_lib soar)

target_include_directories(${PROJECT_NAME} PUBLIC
    SoarHelpers
    TestHarness
    SoarUnitTest
)

install(TARGETS ${PROJECT_NAME} test_external_lib
    DESTINATION bin)

install(DIRECTORY SoarTestAgents DESTINATION ${CMAKE_INSTALL_PREFIX}/SoarUnitTests/)
# enable_testing()
# TESTS FROM CI:
if(BUILD_TESTING)
add_test(NAME test_ci COMMAND test_soar -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton)
add_test(NAME test_soar0 COMMAND test_soar -t AgentTest)
add_test(NAME test_soar1 COMMAND test_soar -t AliasTest)
add_test(NAME test_soar2 COMMAND test_soar -t BasicTests)
add_test(NAME test_soar3 COMMAND test_soar -t BuiltinRHSTests)
add_test(NAME test_soar4 COMMAND test_soar -t ChunkingDemoTests)
add_test(NAME test_soar5 COMMAND test_soar -t ChunkingTests)
add_test(NAME test_soar6 COMMAND test_soar -t EpMemFunctionalTests)
add_test(NAME test_soar7 COMMAND test_soar -t ElementXMLTest)
add_test(NAME test_soar8 COMMAND test_soar -t ExternalLibraryTest)
add_test(NAME test_soar9 COMMAND test_soar -t FullTests)
add_test(NAME test_soar10 COMMAND test_soar -t FullTestsClientThreadFullyOptimized)
add_test(NAME test_soar11 COMMAND test_soar -t FullTestsClientThread)
add_test(NAME test_soar12 COMMAND test_soar -t FunctionalTests)
add_test(NAME test_soar13 COMMAND test_soar -t IOTests)
add_test(NAME test_soar14 COMMAND test_soar -t MiscTests)
add_test(NAME test_soar15 COMMAND test_soar -t MultiAgentTest)
add_test(NAME test_soar16 COMMAND test_soar -t SMemFunctionalTests)
add_test(NAME test_soar17 COMMAND test_soar -t SvsTests)
add_test(NAME test_soar18 COMMAND test_soar -t TokenizerTest)
add_test(NAME test_soar19 COMMAND test_soar -t WmaFunctionalTests)
endif

if(ENABLE_TEST_COVERAGE)
  target_compile_options(soar PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(soar PUBLIC -fprofile-arcs -ftest-coverage)
endif()
