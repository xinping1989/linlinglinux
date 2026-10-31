cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

# Set GLFW options BEFORE fetching
set(GLFW_USE_WAYLAND ON CACHE BOOL "Use Wayland")
set(GLFW_USE_X11 OFF CACHE BOOL "Disable X11")
set(GLFW_USE_COCOA OFF CACHE BOOL "Disable Cocoa")
set(GLFW_USE_WIN32 OFF CACHE BOOL "Disable Win32")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable docs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples")

# Use local ImGui from imgui/ directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

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

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${IMGUI_DIR}
)

# Download GLFW and ImGui backends
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui_backends
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui_backends)

# Add backend sources and includes
set(IMGUI_BACKEND_SOURCES
    ${imgui_backends_SOURCE_DIR}/examples/imgui_impl_glfw.cpp
    ${imgui_backends_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
)

include_directories(
    ${imgui_backends_SOURCE_DIR}/examples
    ${glfw_SOURCE_DIR}/include
)

add_executable(danarc ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

# Build GLFW from source
add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

find_package(OpenGL REQUIRED)

# Link libraries
target_link_libraries(danarc
    glfw
    OpenGL::GL
    m
    stdc++fs
    dl
    pthread
)

target_compile_options(danarc PRIVATE -Wall -O2)
