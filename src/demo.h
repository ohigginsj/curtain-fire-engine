#pragma once

#include "game.h"

// NOTE: Everything the engine has to know about the user script goes here.

struct colors
{
    color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
    color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
    color Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
    color Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
    color Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
    color Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
    color Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
    color Purple = Color(1.0f, 0.0f, 1.0f, 1.0f);
    color Orange = Color(1.0f, 0.4f, 0.0f, 1.0f);
} Colors;

color ColorList[] = {
    Colors.Red,
    Colors.Green,
    Colors.Blue,
    Colors.Yellow,
    Colors.Cyan,
    Colors.Purple,
    Colors.Orange,
    Colors.White,
    Colors.Black,
};

enum ship_id
{
    ShipId_Hind,

    ShipId_Count
};

enum weapon_id
{
    WeaponId_MachineGun,
    WeaponId_Minigun,

    WeaponId_Count
};

enum projectile_sprite_id
{
    ProjectileSprite_Ball_SS,
    ProjectileSprite_Ball_S,
    ProjectileSprite_Ball_M,
    ProjectileSprite_Ball_M_Add,
    ProjectileSprite_Needle,
    ProjectileSprite_Rice_S,
    ProjectileSprite_Ice,
    ProjectileSprite_Bullet,
    ProjectileSprite_Rice_M,
    ProjectileSprite_Kunai,
    ProjectileSprite_Scale,
    ProjectileSprite_Bill,
    ProjectileSprite_Star_S,
    ProjectileSprite_Star_M,
    ProjectileSprite_Laser,
    ProjectileSprite_Coin,
    ProjectileSprite_Ball_Ring,
    ProjectileSprite_Ball_Glow,
    ProjectileSprite_Butterfly,
    ProjectileSprite_Dagger_Youmu,
    ProjectileSprite_Dagger_Kouma,
    ProjectileSprite_Ball_L,
    ProjectileSprite_Delay,

    ProjectileSprite_Count
};

enum texture_id
{
    TextureId_Background,
    TextureId_Weapon_MachineGun,
    TextureId_Weapon_Minigun,
    TextureId_Enemy_Scarab,
    TextureId_Hind_Body,
    TextureId_Hind_Propeller,
    TextureId_BulletMap,
    TextureId_MuzzleFlash,
    TextureId_Explosion_0,
    TextureId_Explosion_1,
    TextureId_Explosion_2,
    TextureId_Graze,
    TextureId_Petal,
    TextureId_Burst,

    // Turn into a map
    TextureId_Particle_Smoke,

    TextureId_Count
};

enum animation_spec_id
{
    AnimationSpecId_Propeller,
    AnimationSpecId_Propeller_Tracer,
    AnimationSpecId_Explosion_0,
    AnimationSpecId_Explosion_1,
    AnimationSpecId_Explosion_2,

    AnimationSpecId_Count
};

// Sound Groups require their sounds to be contiguous in this enum.
enum sound_id
{
    SoundId_ShotA,
    SoundId_ShotB,
    SoundId_ShotC,
    SoundId_ShotD,
    SoundId_ShotE,

    SoundId_Freeze,

    SoundId_LaserA,
    SoundId_LaserB,
    SoundId_LaserC,

    SoundId_ChargeA,
    SoundId_ChargeB,
    SoundId_ChargeC,

    SoundId_Graze,

    //SoundId_ShootA,
    //SoundId_ShootB,
    //SoundId_ShootC,
    //SoundId_ShootD,

    //SoundId_ImpactA,
    //SoundId_ImpactB,

    SoundId_Count
};

enum sound_group_id
{
    //SoundGroupId_Shoot,
    //SoundGroupId_Impact,

    SoundGroupId_Count
};

void UpdateStage1(game_state* GameState, int32 Time);

#include "demo.cpp"
#include "demo_stage_1.cpp"
