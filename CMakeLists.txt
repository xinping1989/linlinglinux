cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

# Use local ImGui from imgui/ directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Download ImGui backend implementations
include(FetchContent)
FetchContent_Declare(
    imgui_backends
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui_backends)

# Your source files
file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
)

# ImGui core sources from local directory
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
)

# Backend sources
set(IMGUI_BACKEND_SOURCES
    ${imgui_backends_SOURCE_DIR}/examples/imgui_impl_glfw.cpp
    ${imgui_backends_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${IMGUI_DIR}
    ${imgui_backends_SOURCE_DIR}/examples
)

add_executable(danarc ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(danarc
    glfw
    OpenGL::GL
    X11
    m
    stdc++fs
    dl
)

target_compile_options(danarc PRIVATE -Wall -O2)
