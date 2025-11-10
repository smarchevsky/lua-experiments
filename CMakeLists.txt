cmake_minimum_required(VERSION 3.16)

project(lua-experiments LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#${CMAKE_SOURCE_DIR}/*.h
FILE(GLOB_RECURSE ALL_HEADERS "src/*.h" "src/*.hpp")
FILE(GLOB_RECURSE ALL_CPP "src/*.c" "src/*.cpp")

add_executable(${PROJECT_NAME} ${ALL_CPP} ${ALL_HEADERS}) # "main.cpp"

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw GL lua)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")
