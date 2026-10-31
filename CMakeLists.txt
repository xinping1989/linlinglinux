cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

# Use local ImGui from imgui/ directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Download GLFW using FetchContent - disable X11 requirement
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable docs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples")
set(GLFW_USE_WAYLAND ON CACHE BOOL "Use Wayland")
set(GLFW_USE_X11 OFF CACHE BOOL "Disable X11")
FetchContent_MakeAvailable(glfw)

# Download ImGui backends
FetchContent_Declare(
    imgui_backends
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui_backends)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${IMGUI_DIR}
    ${imgui_backends_SOURCE_DIR}/examples
    ${glfw_SOURCE_DIR}/include
)

file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
)

# ImGui core sources from local imgui/ directory
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

add_executable(danarc ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

find_package(OpenGL REQUIRED)

# Build GLFW as part of the project
add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

target_link_libraries(danarc
    glfw
    OpenGL::GL
    m
    stdc++fs
    dl
)

target_compile_options(danarc PRIVATE -Wall -O2)
