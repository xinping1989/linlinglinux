// dear imgui: Platform Binding for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, DirectX11, Metal..)

// Implementations of these functions are optional and can be removed if not used/needed.
// This file is designed to be included *after* imgui.cpp and imgui.h.

// Note: GLFW callbacks are not thread-safe. If you use this in a multi-threaded context, you may need
// to add mutex locks around the callback functions.

#include "imgui.h"
#include "imgui_impl_glfw.h"

// GLFW includes
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // For GLFWNativeWin32Window
#endif

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

// Data
static GLFWwindow*     g_Window = NULL;
static double         g_Time = 0.0;
static bool            g_MouseJustPressed[ImGuiMouseButton_COUNT] = { false };
static GLFWcursor*     g_MouseCursors[ImGuiMouseCursor_COUNT] = { NULL };
static bool            g_InstalledCallbacks = false;

// Chain GLFW callbacks (so we can handle input from our side)
static GLFWwindowfocusfun      g_PrevUserCallbackWindowFocus = NULL;
static GLFWcursorenterfun     g_PrevUserCallbackCursorEnter = NULL;
static GLFWcursorposfun       g_PrevUserCallbackCursorPos = NULL;
static GLFWmousebuttonfun     g_PrevUserCallbackMouseButton = NULL;
static GLFWscrollfun         g_PrevUserCallbackScroll = NULL;
static GLFWkeyfun             g_PrevUserCallbackKey = NULL;
static GLFWcharfun            g_PrevUserCallbackChar = NULL;
static GLFWmonitorfun         g_PrevUserCallbackMonitor = NULL;

static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

static void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (g_PrevUserCallbackMouseButton && g_PrevUserCallbackMouseButton != ImGui_ImplGlfw_MouseButtonCallback)
        g_PrevUserCallbackMouseButton(window, button, action, mods);

    if (action == GLFW_PRESS && button >= 0 && button < ImGuiMouseButton_COUNT)
        g_MouseJustPressed[button] = true;
}

static void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_PrevUserCallbackScroll && g_PrevUserCallbackScroll != ImGui_ImplGlfw_ScrollCallback)
        g_PrevUserCallbackScroll(window, xoffset, yoffset);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

static void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (g_PrevUserCallbackKey && g_PrevUserCallbackKey != ImGui_ImplGlfw_KeyCallback)
        g_PrevUserCallbackKey(window, key, scancode, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
    if (g_PrevUserCallbackChar && g_PrevUserCallbackChar != ImGui_ImplGlfw_CharCallback)
        g_PrevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

static void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event)
{
    if (g_PrevUserCallbackMonitor && g_PrevUserCallbackMonitor != ImGui_ImplGlfw_MonitorCallback)
        g_PrevUserCallbackMonitor(monitor, event);

    if (event == GLFW_CONNECTED)
    {
        // Monitor connected: update DPI scale
        ImGui_ImplGlfw_GetDpiScaleForCurrentMonitor();
    }
}

static void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused)
{
    if (g_PrevUserCallbackWindowFocus && g_PrevUserCallbackWindowFocus != ImGui_ImplGlfw_WindowFocusCallback)
        g_PrevUserCallbackWindowFocus(window, focused);
}

static void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered)
{
    if (g_PrevUserCallbackCursorEnter && g_PrevUserCallbackCursorEnter != ImGui_ImplGlfw_CursorEnterCallback)
        g_PrevUserCallbackCursorEnter(window, entered);
}

static void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (g_PrevUserCallbackCursorPos && g_PrevUserCallbackCursorPos != ImGui_ImplGlfw_CursorPosCallback)
        g_PrevUserCallbackCursorPos(window, x, y);

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantSetMousePos)
    {
        glfwSetCursorPos(window, (double)io.MousePos.x, (double)io.MousePos.y);
    }
    else
    {
        io.MousePos = ImVec2((float)x, (float)y);
    }
}

void ImGui_ImplGlfw_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        // Show OS mouse cursor
        glfwSetCursor(g_Window, g_MouseCursors[imgui_cursor] ? g_MouseCursors[imgui_cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void ImGui_ImplGlfw_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Get gamepad
    GLFWgamepadstate state;
    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
        return;

    // Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON)       do { if (state.buttons[BUTTON] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; } while (0)
#define MAP_ANALOG(NAV_NO, AXIS, VALUE)  do { float v = (float)state.axes[AXIS]; v = (v > 0.1f) ? v : 0.0f; io.NavInputs[NAV_NO] = v; } while (0)
    MAP_BUTTON(ImGuiNavInput_Activate,     GLFW_GAMEPAD_BUTTON_SOUTH);              // A / Cross
    MAP_BUTTON(ImGuiNavInput_Cancel,       GLFW_GAMEPAD_BUTTON_EAST);               // B / Circle
    MAP_BUTTON(ImGuiNavInput_Menu,         GLFW_GAMEPAD_BUTTON_WEST);               // X / Square
    MAP_BUTTON(ImGuiNavInput_Input,        GLFW_GAMEPAD_BUTTON_WEST);               // X / Square
    MAP_BUTTON(ImGuiNavInput_DpadLeft,     GLFW_GAMEPAD_BUTTON_DPAD_LEFT);           // D-Pad Left
    MAP_BUTTON(ImGuiNavInput_DpadRight,    GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);          // D-Pad Right
    MAP_BUTTON(ImGuiNavInput_DpadUp,       GLFW_GAMEPAD_BUTTON_DPAD_UP);             // D-Pad Up
    MAP_BUTTON(ImGuiNavInput_DpadDown,     GLFW_GAMEPAD_BUTTON_DPAD_DOWN);           // D-Pad Down
    MAP_BUTTON(ImGuiNavInput_LStickLeft,   GLFW_GAMEPAD_BUTTON_THUMB_LEFT);          // L3
    MAP_BUTTON(ImGuiNavInput_LStickRight,  GLFW_GAMEPAD_BUTTON_THUMB_LEFT);          // L3
    MAP_BUTTON(ImGuiNavInput_LStickUp,     GLFW_GAMEPAD_BUTTON_THUMB_LEFT);          // L3
    MAP_BUTTON(ImGuiNavInput_LStickDown,   GLFW_GAMEPAD_BUTTON_THUMB_LEFT);          // L3
    MAP_ANALOG(ImGuiNavInput_LStickLeft,   GLFW_GAMEPAD_AXIS_LEFT_X,  -1.0f);
    MAP_ANALOG(ImGuiNavInput_LStickRight,  GLFW_GAMEPAD_AXIS_LEFT_X,   1.0f);
    MAP_ANALOG(ImGuiNavInput_LStickUp,     GLFW_GAMEPAD_AXIS_LEFT_Y,  -1.0f);
    MAP_ANALOG(ImGuiNavInput_LStickDown,   GLFW_GAMEPAD_AXIS_LEFT_Y,   1.0f);
    MAP_BUTTON(ImGuiNavInput_RStickLeft,   GLFW_GAMEPAD_BUTTON_THUMB_RIGHT);         // R3
    MAP_BUTTON(ImGuiNavInput_RStickRight,  GLFW_GAMEPAD_BUTTON_THUMB_RIGHT);         // R3
    MAP_BUTTON(ImGuiNavInput_RStickUp,     GLFW_GAMEPAD_BUTTON_THUMB_RIGHT);         // R3
    MAP_BUTTON(ImGuiNavInput_RStickDown,   GLFW_GAMEPAD_BUTTON_THUMB_RIGHT);         // R3
    MAP_ANALOG(ImGuiNavInput_RStickLeft,   GLFW_GAMEPAD_AXIS_RIGHT_X, -1.0f);
    MAP_ANALOG(ImGuiNavInput_RStickRight,  GLFW_GAMEPAD_AXIS_RIGHT_X,  1.0f);
    MAP_ANALOG(ImGuiNavInput_RStickUp,     GLFW_GAMEPAD_AXIS_RIGHT_Y, -1.0f);
    MAP_ANALOG(ImGuiNavInput_RStickDown,   GLFW_GAMEPAD_AXIS_RIGHT_Y,  1.0f);
    MAP_BUTTON(ImGuiNavInput_FocusPrev,    GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);         // L1
    MAP_BUTTON(ImGuiNavInput_FocusNext,    GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);        // R1
    MAP_BUTTON(ImGuiNavInput_TweakSlow,    GLFW_GAMEPAD_BUTTON_LEFT_TRIGGER);        // L2
    MAP_BUTTON(ImGuiNavInput_TweakFast,    GLFW_GAMEPAD_BUTTON_RIGHT_TRIGGER);       // R2
#undef MAP_BUTTON
#undef MAP_ANALOG
}

void ImGui_ImplGlfw_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to account for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

    // Setup mouse inputs (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled via the custom callbacks)
    if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
    {
        double mouse_x, mouse_y;
        glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    for (int i = 0; i < ImGuiMouseButton_COUNT; i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(g_Window, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    // Update OS mouse cursor with the cursor requested by imgui
    ImGui_ImplGlfw_UpdateMouseCursor();

    // Gamepad navigation
    ImGui_ImplGlfw_UpdateGamepads();

    // DPI scale
    ImGui_ImplGlfw_GetDpiScaleForCurrentMonitor();
}

static void ImGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks)
{
    g_Window = window;

    // Setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_glfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;            // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;     // We can create multi-viewports on the Platform side (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;  // We can call io.AddMouseViewportEvent() with correct coordinates (optional)

    // Keyboard mapping
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    // Set clipboard callbacks
    io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    io.ClipboardUserData = g_Window;

    // Mouse cursors
    g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_No] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    // Set platform dependent data
    ImGuiViewPortP* main_viewport = (ImGuiViewPortP*)(void*)ImGui::GetMainViewport();
    main_viewport->PlatformHandle = window;
    main_viewport->PlatformHandleRaw = glfwGetWin32Window(window);

    // Chain callbacks
    if (install_callbacks)
    {
        g_PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
        g_PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
        g_PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
        g_PrevUserCallbackMouseButton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
        g_PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
        g_PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
        g_PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
        g_PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
        g_InstalledCallbacks = true;
    }

    // Update DPI scale
    ImGui_ImplGlfw_GetDpiScaleForCurrentMonitor();
}

bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks)
{
    ImGui_ImplGlfw_Init(window, install_callbacks);
    return true;
}

bool ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks)
{
    ImGui_ImplGlfw_Init(window, install_callbacks);
    return true;
}

bool ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks)
{
    ImGui_ImplGlfw_Init(window, install_callbacks);
    return true;
}

void ImGui_ImplGlfw_Shutdown()
{
    // Restore callbacks
    if (g_InstalledCallbacks)
    {
        glfwSetWindowFocusCallback(g_Window, g_PrevUserCallbackWindowFocus);
        glfwSetCursorEnterCallback(g_Window, g_PrevUserCallbackCursorEnter);
        glfwSetCursorPosCallback(g_Window, g_PrevUserCallbackCursorPos);
        glfwSetMouseButtonCallback(g_Window, g_PrevUserCallbackMouseButton);
        glfwSetScrollCallback(g_Window, g_PrevUserCallbackScroll);
        glfwSetKeyCallback(g_Window, g_PrevUserCallbackKey);
        glfwSetCharCallback(g_Window, g_PrevUserCallbackChar);
        glfwSetMonitorCallback(g_PrevUserCallbackMonitor);
        g_InstalledCallbacks = false;
    }

    // Destroy mouse cursors
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(g_MouseCursors[cursor_n]);

    g_Window = NULL;
}

float ImGui_ImplGlfw_GetDpiScaleForMonitor(void* monitor)
{
    float xscale, yscale;
    glfwGetMonitorContentScale((GLFWmonitor*)monitor, &xscale, &yscale);
    return xscale;
}

float ImGui_ImplGlfw_GetDpiScaleForCurrentMonitor()
{
    if (g_Window == NULL)
        return 1.0f;

    GLFWmonitor* monitor = glfwGetWindowMonitor(g_Window);
    if (monitor == NULL)
        monitor = glfwGetPrimaryMonitor();

    float scale = ImGui_ImplGlfw_GetDpiScaleForMonitor(monitor);
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f / scale;
    return scale;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
