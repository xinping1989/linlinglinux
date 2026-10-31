cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

# Use FetchContent to get all dependencies
include(FetchContent)

# Download GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW docs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples")
FetchContent_MakeAvailable(glfw)

# Download ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# Get backend implementations from ImGui examples
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# Also include local ImGui files from the repository
set(LOCAL_IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/examples
    ${glfw_SOURCE_DIR}/include
    ${LOCAL_IMGUI_DIR}
)

file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
)

# ImGui sources - use fetched version
set(IMGUI_CORE_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# ImGui backend sources
set(IMGUI_BACKEND_SOURCES
    ${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
)

# GLFW sources
set(GLFW_SOURCES
    ${glfw_SOURCE_DIR}/src/glfw_config.h
    ${glfw_SOURCE_DIR}/src/context.c
    ${glfw_SOURCE_DIR}/src/init.c
    ${glfw_SOURCE_DIR}/src/input.c
    ${glfw_SOURCE_DIR}/src/monitor.c
    ${glfw_SOURCE_DIR}/src/vulkan.c
    ${glfw_SOURCE_DIR}/src/window.c
)

add_executable(danarc ${SOURCES} ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES} ${glfw_SOURCES})

find_package(OpenGL REQUIRED)

target_link_libraries(danarc
    OpenGL::GL
    X11
    m
    stdc++fs
    dl
    pthread
)

target_compile_options(danarc PRIVATE -Wall -O2)
