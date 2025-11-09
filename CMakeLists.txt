cmake_minimum_required(VERSION 3.16)

project(lua-experiments LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")

add_executable(lua-experiments main.cpp
    cpp_math.h
    cpp_math.cpp)

target_link_libraries(lua-experiments lua)

include(GNUInstallDirs)
install(TARGETS lua-experiments
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
