// dear imgui: Platform Binding for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, DirectX11, Metal..)

// Implementations of these functions are optional and can be removed if not used/needed.
// This file is designed to be included *after* imgui.cpp and imgui.h.

#pragma once

#include "imgui.h"

IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplGlfw_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplGlfw_NewFrame();

// GLFW callbacks (installed by default if you enable install_callbacks)
IMGUI_IMPL_API void     ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);
IMGUI_IMPL_API void     ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
IMGUI_IMPL_API void     ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);

// DPI scale (for retina / high-DPI displays)
// (Optional) If your backend supports it, you may scale font/texture by this value.
// You can also set ImGui::GetIO().FontGlobalScale = 1.0f / ImGui_ImplGlfw_GetDpiScaleForMonitor() in your code.
IMGUI_IMPL_API float    ImGui_ImplGlfw_GetDpiScaleForMonitor(void* monitor);
IMGUI_IMPL_API float    ImGui_ImplGlfw_GetDpiScaleForCurrentMonitor();

