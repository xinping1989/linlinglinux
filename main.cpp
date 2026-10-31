#include <atomic>
#include <filesystem>
#include <iostream>
#include <vector>
#include <cmath>
#include <csignal>
#include <thread>
#include <variant>

#include "gamestate/GameState.h"
#include "draw/ESP.h"
#include "memory/memory.h"

inline std::atomic<bool> g_Running(true);
void SignalHandler(int signum) {g_Running = false;}

int main() {
    std::signal(SIGINT, SignalHandler);
    if (!InitOverlay()) {
        std::cerr << "[-] Failed to init Overlay (GLFW/Wayland)" << std::endl;
        return 1;
    }

    ProcessId = FindGamePID();
    GameState gs = GameState();
    ESPInit();

    if (isDebugMode) {
        std::cout << "\n=== VTable Finder Mode ENABLED ===" << std::endl;
        std::cout << "1. Look at your player - the vtable shown is PLAYER" << std::endl;
        std::cout << "2. Look at ARCs - the vtable shown is ARC" << std::endl;
        std::cout << "3. Look at pickups - the vtable shown is PICKUP" << std::endl;
        std::cout << "4. Look at loot containers - the vtable shown is SEARCH" << std::endl;
        std::cout << "5. Look at camera - the vtable shown is CAMMANAGER" << std::endl;
        std::cout << "6. VTables are logged to: " << VTABLE_LOG_FILE << std::endl;
        std::cout << "7. Press Ctrl+C to exit and see the summary" << std::endl;
    }

    while (g_Running && !glfwWindowShouldClose(window)) {
        gsRet gsr = gs.tick();

        RenderBegin();

        char buf[128];
        sprintf(buf, "Entities: %lu | Debug: %s | VTable Finder: %s",
                gsr.entities.size(),
                isDebugMode ? "ON" : "OFF",
                enableVTableFinder ? "ON" : "OFF");
        DrawTextImGui(10, 10, IM_COL32(255, 0, 0, 255), buf);

        if (isDebugMode && enableVTableFinder) {
            DrawTextImGui(10, 30, COLOR::white, "VTable Finder Active - Look at entities within 400m");
            DrawTextImGui(10, 50, COLOR::green, "GREEN = Likely PLAYER");
            DrawTextImGui(10, 70, COLOR::orange, "ORANGE = Likely ARC");
            DrawTextImGui(10, 90, COLOR::blue, "BLUE = Far entity");
            DrawTextImGui(10, 110, COLOR::red, "RED = Dead entity");
            DrawTextImGui(10, 130, COLOR::white, "VTables logged to: /tmp/danarc_vtables.log");
        }

        DrawESP(gsr.entities, gsr.vm, gsr.localPlayerTeam);
        if (config::enableRadar) DrawRadar(gsr.entities, gsr.vm, gsr.localPlayerTeam);

        RenderEnd();
    }

    std::cout << "[+] Destructing Window" << std::endl;

    if (ImGui::GetCurrentContext()) {
        if (ImGui::GetIO().BackendRendererUserData)
            ImGui_ImplOpenGL3_Shutdown();
        if (ImGui::GetIO().BackendPlatformUserData)
            ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (window) {
        glfwHideWindow(window);
        glfwDestroyWindow(window);
    }

    SaveVTableResults();
    return 0;
}
