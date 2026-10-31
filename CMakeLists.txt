cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
)

add_executable(danarc ${SOURCES})

target_link_libraries(danarc
    glfw
    OpenGL::GL
    X11
    m
    stdc++fs
)

target_compile_options(danarc PRIVATE -Wall -O2)
