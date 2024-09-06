cmake_minimum_required(VERSION 3.22)
project(test_soar
  LANGUAGES CXX)

file(GLOB_RECURSE SOURCES
    TestHarness/*.cpp
    SoarHelpers/*.cpp
    # SoarUnitTests/*.cpp
    # SoarUnitTests/wma/*.cpp
    )

file(GLOB_RECURSE HEADERS
    TestHarness/*.hpp
    SoarHelpers/*.hpp
    # SoarUnitTests/*.hpp
    # SoarUnitTests/wma/*.hpp
    )

add_library(test_lib SHARED ${SOURCES} ${HEADERS})
set_target_properties(test_lib
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}")

# add_executable(${PROJECT_NAME} testMain.cpp)
# target_link_libraries(${PROJECT_NAME} soar test_lib)

# install(TARGETS ${PROJECT_NAME}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY SoarTestAgents DESTINATION ${CMAKE_INSTALL_PREFIX}/SoarUnitTests/)
