#pragma once

#include "game.h"

struct player;
struct ship;
struct weapon;
struct bomb;
struct projectile;
struct particle;
struct enemy;
struct boss;
struct menu_grid;

typedef void (*world_update_function)(game_state* GameState, int32 Time);

struct scrolling_background
{
    texture_id TextureId;
    int32 TextureRepeatY;
    real32 ScrollSpeed;
    real32 SourceY;
};

enum control_mode
{
    ControlMode_Normal,
    ControlMode_DeathAnimation,
    ControlMode_DeathMenu,
    ControlMode_FatalCollision,
};

struct game_mode_world
{
    memory_zone Memory;

    int32 Timer;
    world_update_function UpdateFunction;

    bool32 Paused;
    bool32 DebugOverlay;

    menu_grid* RestartMenu;

    int32 FatalCollisionTime;
    int32 DeathAnimationTime;
    control_mode ControlMode;

    player* Player;
    bomb* Bomb;

    real32 BulletBounds;

    uint32 ProjectileCount;
    projectile* Projectiles;

    uint32 ParticleCount;
    particle* Particles;

    bool32 FreezeBossTime;
    boss* Boss;
    uint32 EnemyCount;
    enemy* Enemies;

    // TODO: Better place for these?
    ship* Ships;
    weapon* Weapons;

    scrolling_background* ScrollingBackground;
};

void
SetBulletBounds(game_state* GameState, real32 Bounds)
{
    game_mode_world* World = GameState->World;
    World->BulletBounds = Bounds;
}

void
ResetBulletBounds(game_mode_world* World)
{
    World->BulletBounds = BulletBoundsDefault;
}

// API
void
SetUpdateFunction(game_state* GameState,
                  world_update_function Function)
{
    game_mode_world* World = GameState->World;
    World->UpdateFunction = Function;
}
