#pragma once

#include <glad/glad.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_mixer.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "game_math.h"
#include "color.h"

////////////////////////////////////////////
// MEMORY
/////////////////////////////////////////////
struct memory_zone
{
   size_t Size;
   uint8* BaseAddress;
   size_t Used;

   int32 TempCount;
};

struct temporary_memory
{
    memory_zone* Memory;
    size_t OriginalUsed;
};

inline void
InitializeMemoryZone(memory_zone* Zone, size_t Size, void* BaseAddress)
{
    Zone->Size = Size;
    Zone->BaseAddress = (uint8*)BaseAddress;
    Zone->Used = 0;
    Zone->TempCount = 0;
}

#define PushStruct(Zone, type) (type*)PushSize(Zone, sizeof(type))
#define PushArray(Zone, Count, type) (type*)PushSize(Zone, (Count)*sizeof(type))
inline void*
PushSize(memory_zone* Zone, size_t Size)
{
    Assert("Memory Zone Overflow.", (Zone->Used + Size) <= Zone->Size);
    void* Address = Zone->BaseAddress + Zone->Used;
    Zone->Used += Size;
    return(Address);
}

inline size_t
GetMemoryZoneRemaining(memory_zone* MemoryZone)
{
    return MemoryZone->Size - MemoryZone->Used;
}

inline void
DivideMemoryZone(memory_zone* Result,
                 memory_zone* ParentMemory,
                 size_t Size)
{
    Result->Size = Size;
    Result->BaseAddress = (uint8*)PushSize(ParentMemory, Size);
    Result->Used = 0;
}

inline void
ClearMemoryZone(memory_zone* Zone)
{
    InitializeMemoryZone(Zone, Zone->Size, Zone->BaseAddress);
}

inline temporary_memory
BeginTemporaryMemory(memory_zone* Memory)
{
    temporary_memory Result;
    Result.Memory = Memory;
    Result.OriginalUsed = Memory->Used;
    ++Memory->TempCount;

    return Result;
}

inline void
EndTemporaryMemory(temporary_memory* TemporaryMemory)
{
    memory_zone* Memory = TemporaryMemory->Memory;
    Assert("Memory has decreased... somehow.", TemporaryMemory->OriginalUsed <= Memory->Used);
    Memory->Used = TemporaryMemory->OriginalUsed;
    Assert("Begin / End Temporary Memory mismatch.", Memory->TempCount > 0);
    --Memory->TempCount;
}

inline void
CheckMemoryZone(memory_zone* Memory)
{
    Assert("Temporary Memory not freed.", Memory->TempCount == 0);
}

struct button_state
{
    bool32 Pressed;
    bool32 Down;
};

struct resize_window_command
{
    bool32 Active;
    int32 WindowScale;
};

struct game_input
{
    button_state Esc;
    button_state Space;
    button_state Enter;

    button_state K;
    button_state Q;
    button_state R;
    button_state I;
    button_state Z;
    button_state X;
    button_state P;
    button_state N;
    button_state W;
    button_state A;
    button_state S;
    button_state D;

    button_state ArrowUp;
    button_state ArrowDown;
    button_state ArrowLeft;
    button_state ArrowRight;

    button_state Control;
    button_state Shift;

    bool32 MousePressed_Left;
    bool32 MousePressed_Right;
    int32 MousePositionX;
    int32 MousePositionY;

    resize_window_command ResizeWindow;
    bool32 ReloadedGameLibrary;

    real32 DeltaTime;
    real32 LastFrameFPS;
};

// TODO: Group these up
const int32 GameUpdateHz = 60;

const int32 DebugPanelHeight = 64;
const int32 LogicalWindowSizeX = 348;
const int32 LogicalWindowSizeY = 448 + DebugPanelHeight;
const real32 RenderPadding = 32.0f;

const uint32 ProjectileCountMax = 4096;
const uint32 ParticleCountMax = 1024;
const uint32 EnemyCountMax = 32;

const v2 LooseLaserCollisionFactor = V2(0.2f, 0.7f);
const v2 StraightLaserCollisionFactor = V2(0.4f, 1.0f);
const int32 ProjectileFadeTime = 12;

const real32 BulletBoundsDefault = 50.0f;

const int32 Controller_AnalogDeadZone = 8000;

const bool32 EnableController = false;

const real32 Debug_ObjectCounterWarningPercentage = 0.75;

// NOTE: These are for checking user-defined layers
const int32 RenderZone_Background = 100;
const int32 RenderZone_World = 80;
const int32 RenderZone_UI = 20;
const int32 RenderZone_Top = 0;

// NOTE: Layer presets for the engine's rendering
const int32 EngineLayer_Background = 100;

const int32 EngineLayer_BossInfo = 60;
const int32 EngineLayer_BossDefault = 60;
const int32 EngineLayer_PlayerDefault = 55;
const int32 EngineLayer_EnemyDefault = 50;
const int32 EngineLayer_Bomb = 40;

const int32 EngineLayer_DebugDarken = 15;
const int32 EngineLayer_DebugCollisionInfo = 10;
const int32 EngineLayer_DebugPanel = 5;
const int32 EngineLayer_DeathMenu = 2;
const int32 EngineLayer_DebugInfo = 0;
const int32 EngineLayer_MainMenu = 0;
const int32 EngineLayer_RestartMenu = 0;

enum dualshock3_buttons
{
    DualShock3_Select,
    DualShock3_L3,
    DualShock3_R3,
    DualShock3_Start,
    DualShock3_Up,
    DualShock3_Right,
    DualShock3_Down,
    DualShock3_Left,
    DualShock3_L2,
    DualShock3_R2,
    DualShock3_L1,
    DualShock3_R1,
    DualShock3_Triangle,
    DualShock3_Circle,
    DualShock3_X,
    DualShock3_Square,
};

struct renderer;
struct random_mt;
struct shader;

struct sound;
struct sound_group;
struct texture;
struct animation_spec;
struct bullet_sprite;

struct game_mode_main_menu;
struct game_mode_world;

enum game_mode
{
    GameMode_None,

    GameMode_MainMenu,
    GameMode_World,
};

struct assets
{
    sound* Sounds;
    sound_group* SoundGroups;

    texture* Textures;
    animation_spec* Animations;
    bullet_sprite* BulletSprites;
};

struct transient_state
{
    bool32 IsInitialized;

    memory_zone Memory;
};

struct game_state
{
    bool32 IsInitialized;

    memory_zone ModeMemory;
    memory_zone AssetMemory;
    memory_zone RenderMemory;

    // TODO: When user script has its own struct to pass around for state,
    // move this there.
    transient_state* TransientState;

    bool32 Running;

    renderer* Renderer;
    shader* Shaders;

    real32 GlobalVolume;

    assets Assets;

    random_mt* Random;

    game_mode GameMode;
    union
    {
        game_mode_main_menu* MainMenu;
        game_mode_world* World;
    };
};

typedef int32 (game_loop_function)(game_memory* GameMemory, game_input* GameInput);

void
SetGameMode(game_state* GameState, game_mode GameMode)
{
    // TODO: Should we clear to zero?
    ClearMemoryZone(&GameState->ModeMemory);
    GameState->GameMode = GameMode;
};

void
Seed(game_state* GameState, uint32 Value)
{
    MT_Seed(GameState->Random, Value);
}

// Range: [0, 1)
inline real32
Random(game_state* GameState)
{
    real32 Result = (real32)MT_Extract(GameState->Random) / (real32)UINT32_MAX;
    return Result;
}

inline real32
RandomRange(game_state* GameState, real32 Max)
{
    real32 Result = Random(GameState) * Max;
    return Result;
}

inline real32
RandomRange(game_state* GameState, real32 Min, real32 Max)
{
    real32 Result = Min + RandomRange(GameState, Max);
    return Result;
}

inline real32
RandomSign(game_state* GameState)
{
    return Random(GameState) < 0.5f ? 1.0f : -1.0f;
}

inline real32
RandomPerturb(game_state* GameState, real32 Value, real32 Pertubation)
{
    real32 Result = Value + (RandomRange(GameState, -Pertubation, 2.0f * Pertubation));
    return Result;
}

inline game_mode_world*
GetWorld(game_state* GameState)
{
    Assert("Game state is not in world mode.", GameState->GameMode == GameMode_World);
    return GameState->World;
}
