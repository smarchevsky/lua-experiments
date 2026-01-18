cmake_minimum_required(VERSION 3.16)

project(lua-experiments LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")



FILE(GLOB_RECURSE ALL_CPP "src/*.c" "src/*.cpp")
add_executable(${PROJECT_NAME} ${ALL_CPP})



target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/3party/
    ${CMAKE_SOURCE_DIR}/src/3party/imgui/
    #${LUA_INCLUDE_DIR}
)


if(NOT WIN32)
target_link_libraries(${PROJECT_NAME} glfw GL lua)
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endif()










