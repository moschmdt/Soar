cmake_minimum_required(VERSION 3.22)
project(gtest_soar
    LANGUAGES CXX)

find_package(GTest REQUIRED)

add_executable(test_identifier_exceptions_gtest UnitTests/IdentifierExceptionTests.cpp)
target_link_libraries(test_identifier_exceptions_gtest
    PRIVATE
    soar_lib
    GTest::gtest
    GTest::gtest_main
)
target_compile_features(test_identifier_exceptions_gtest PUBLIC
    cxx_std_17
)

add_executable(test_operator_explanation_gtest UnitTests/OperatorExplanationTests.cpp)
target_link_libraries(test_operator_explanation_gtest
    PRIVATE
    soar_lib
    GTest::gtest
    GTest::gtest_main
)
target_compile_features(test_operator_explanation_gtest PUBLIC
    cxx_std_17
)

if(BUILD_TESTING)
    add_test(NAME test_identifier_exceptions_gtest COMMAND test_identifier_exceptions_gtest)
    add_test(NAME test_operator_explanation_gtest COMMAND test_operator_explanation_gtest)
endif()
