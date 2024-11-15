cmake_minimum_required(VERSION 3.22)
project(soar_cli
    VERSION 9.6.3
    LANGUAGES CXX)

add_executable(soar_cli soar_cli.cpp)
target_link_libraries(soar_cli soar::soar)

install(TARGETS soar_cli
    DESTINATION bin)
