#ifndef DANARC_CONFIG_H
#define DANARC_CONFIG_H
#include <cstdint>

//-------------------Config-------------------// btw m stands of meters
namespace config {
    //ESP Preferences
    inline bool makeLootESPTranslucent = true;
    inline bool makeDeadBotESPTranslucent = true;
    inline bool makeDeadPlayerESPTranslucent = true;

    //ESP Distance Config
    inline double maxPlayerDist = 400/*<-Dist in m*/;
    inline double maxArcDist = 150  /*<-Dist in m*/ ;
    inline double maxLootDist = 40 /*<-Dist in m*/  ;

    //If You want Radar
    inline bool enableRadar = true;
    inline float radarCenterX = 150; //coords px
    inline float radarCenterY = 150; //coords px
    inline float radarRadius = 100;  //size px
    inline float scale = 200.0f; //meters
    inline float selfDot = false;

    //Set this to your monitor resolution
    inline int SCREEN_H = 1440 /*<-Make you monitor height*/ * 0.94/*just trust me, this helps with distortion around screen edge*/;
    inline int SCREEN_W = 2560 /*<-Make you monitor width*/;

    inline const auto OVERLAY_NAME = "Discover Overlay"; //Name of the overlay
}

//For updating the program
inline constexpr bool isDebugMode = true; // ENABLED for vtable finding
//----------------------------------------------

// VTable finding mode - when enabled, will auto-label and log vtables
inline constexpr bool enableVTableFinder = true;
inline constexpr bool logVTablesToFile = true; // Save vtables to a file for easy reference
inline const char* VTABLE_LOG_FILE = "/tmp/danarc_vtables.log";

#define pr inline uintptr_t
#define ptr uintptr_t
#define DBG if(!isDebugMode) {} else

// Level decryption function (for encrypted PersistentLevel pointer)
// From SDK dump: XOR with 0x7DD776A600000000 then byte-swap
inline uintptr_t DecodeLevelPointer(uintptr_t encrypted) {
    uintptr_t xored = encrypted ^ 0x7DD776A600000000ULL;
    // Byte swap for little-endian
    return ((xored & 0xFF00000000000000) >> 56) |
           ((xored & 0x00FF000000000000) >> 40) |
           ((xored & 0x0000FF0000000000) >> 24) |
           ((xored & 0x000000FF00000000) >> 8)  |
           ((xored & 0x00000000FF000000) << 8)  |
           ((xored & 0x0000000000FF0000) << 24) |
           ((xored & 0x000000000000FF00) << 40) |
           ((xored & 0x00000000000000FF) << 56);
}

// NEW: 4-level pointer chain for GWorld -> UWorld
// Chain: [base+0x109bb1c0] -> [+0xa0] -> [+0x248] -> [+0x148] = UWorld*
// Confirmed working from Oct 29, 2026 SDK dump
namespace off {
    pr gworld_chain[] = {0x109bb1c0, 0xa0, 0x248, 0x148};
    pr gworld_chain_size = 4;

    // From UWorld: Encrypted main level pointer (requires DecodeLevelPointer)
    pr PERSISTENT_LEVEL = 0x140;  // Updated Oct 29, 2026: was 0x130

    // From PersistentLevel (ULevel): Actor TArray header
    // Structure: +0x00 = data pointer, +0x08 = count, +0x0C = capacity
    pr ACTORS_PTR = 0x110;  // Updated Oct 29, 2026: was 0x108

    pr ROOT_COMPONENT_PTR = 0x248;  // Unchanged

    // Root Component offsets (updated Oct 29, 2026: shifted +0x40)
    pr POS_PTR = 0x340;  // Updated: was 0x300, now root + 0x340 (RelativeLocation)
    pr COMPONENT_TO_WORLD = 0x320;  // Updated: was 0x2E0, now root + 0x320 (FTransform)

    // Camera offsets (confirmed working)
    pr VIEW_MATRIX = 0x410;  // Updated: was 0xc88, now CameraPOV structure
    pr CAMERA_POV = 0x410;  // CameraPOV: Location (X,Y,Z doubles) + Rotation + FOV
    pr CAMERA_FOV_OFFSET = 0x48;  // FOV is at CameraPOV + 0x48

    // Player/Character offsets (confirmed working)
    pr PLAYER_STATE = 0x3F0;  // PlayerState pointer (valid = is a player character)
    pr LOCAL_PAWN = 0x428;   // From controller: controller + 0x428
    pr CONTROLLER = 0x440;   // OwningController pointer
    pr MESH = 0x450;        // Updated: was 0x428, now 0x450

    // Health offsets (confirmed working)
    pr HEALTH_COMPONENT = 0xD88;  // Updated: was 0xD78
    pr HEALTH_VALUE = 0x6A0;      // HealthComp + 0x6A0 = Current HP (double)

    // "namespace EmbarkCharacterBase"
    pr TEAM_ID = 0x3F0; // Updated: TeamId now at PlayerState offset

    //find under // Class /Script/Engine.Character, "namespace Character"
    // MESH moved to 0x450 above

    //Find by uncommenting getViewMatrix(actor); in GameState.cpp and finding were it matches the pattern.
    // VIEW_MATRIX moved to 0x410 above

    // From PersistentLevel (ULevel) "Level* PersistentLevel;"
    // Known as AActor or Actor Array "struct Actor : public Object". "struct AActor : public UObject"
    // ACTORS_PTR moved to 0x110 above

    // Level encryption: XOR 0x7DD776A600000000 + byte-swap
    pr LEVEL_PTR_OFFSET = 0x140; // Offset from UWorld to encrypted level pointer
    inline constexpr uintptr_t LEVEL_XOR_KEY = 0x7DD776A600000000ULL; // XOR key for level decryption
    inline constexpr bool enableLevelDecryption = true; // Enable/disable level decryption
    pr LEVEL_OWNING_WORLD_OFFSET = 0x138; // OwningWorld offset for verification

    pr IS_DESTROYED = 0x1210; //bIsDestroyed
}

namespace vtabels {
    // Updated vtable addresses - verify these match your live game
    pr ARC = 0x14c7b6d20;
    pr SEARCH = 0x14c7a3000;
    pr PICKUP = 0x14c767570;
    pr PLAYER = 0x14c8ce680;
    pr CAMMANAGER = 0x14bb3c190;
}

#endif //DANARC_CONFIG_H
