// dear imgui: Renderer Backend for OpenGL3+
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32, custom..)

// Implementations of these functions are optional and can be removed if not used/needed.
// This file is designed to be included *after* imgui.cpp and imgui.h.

#pragma once

#include "imgui.h"

IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

// (Optional) Called by Init/Shutdown. You can use this to setup the callbacks for your platform.
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_SetCallbacks();

