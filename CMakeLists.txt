cmake_minimum_required(VERSION 3.10)
project(danarc)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# Download and include Dear ImGui with GLFW and OpenGL3 backends
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# Also fetch the GLFW+OpenGL3 example files which contain the backend implementations
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/gamestate
    ${CMAKE_CURRENT_SOURCE_DIR}/draw
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/examples
)

file(GLOB SOURCES
    "main.cpp"
    "memory/*.cpp"
    "gamestate/*.cpp"
    "draw/*.cpp"
)

# Add ImGui backend sources for GLFW and OpenGL3
set(IMGUI_BACKEND_SOURCES
    ${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
)

add_executable(danarc ${SOURCES} ${imgui_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_link_libraries(danarc
    glfw
    OpenGL::GL
    imgui
    X11
    m
    stdc++fs
)

target_compile_options(danarc PRIVATE -Wall -O2)
