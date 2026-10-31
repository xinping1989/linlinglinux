cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

# Use local ImGui from imgui/ directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${glfw3_INCLUDE_DIRS}
)

file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
    "imgui/*.cpp"
    "imgui/examples/*.cpp"
)

add_executable(danarc ${SOURCES})

target_link_libraries(danarc
    glfw
    OpenGL::GL
    X11
    m
    stdc++fs
    dl
)

target_compile_options(danarc PRIVATE -Wall -O2)
