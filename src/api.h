#pragma once

#include "game_math.h"
#include "demo.h"

// TODO: This should be set by the user script!
int32 WorldSizeX = 348;
int32 WorldSizeY = 448;

real32 BulletSpeedMax = 16.0f;

struct game_state;
struct boss;
struct boss_attack;
struct projectile;
typedef void (*projectile_update_function)(game_state* GameState, projectile* Projectile, int32 Time);
typedef void (*boss_behavior_function)(game_state* GameState, boss* Boss, int32 Time);
typedef void (*world_update_function)(game_state* GameState, int32 Time);

// Projectiles
enum projectile_directionality
{
    Directionality_None,
    Directionality_Angle,
};

enum projectile_type
{
    Projectile_Bullet,
    Projectile_LooseLaser,
    Projectile_StraightLaser,
};

enum target
{
    Target_Player,
    Target_AI,
};

// Rendering
enum texture_filter
{
    TextureFilter_Nearest,
    TextureFilter_Linear,
};

enum blend_mode
{
    BlendMode_Blend,
    BlendMode_Add,
};

enum sprite_type
{
    SpriteType_Static,
    SpriteType_Animation,
};

struct sprite_spec
{
    v2 RenderOffset;

    sprite_type Type;
    union
    {
        texture_id TextureId;
        animation_spec_id AnimationSpecId;
    };
};

inline sprite_spec
Sprite(texture_id TextureId,
       v2 RenderOffset)
{
    sprite_spec Result = {};

    Result.RenderOffset = RenderOffset;
    Result.Type = SpriteType_Static;
    Result.TextureId = TextureId;

    return Result;
}

inline sprite_spec
Sprite_Animated(animation_spec_id AnimationSpecId,
                v2 RenderOffset)
{
    sprite_spec Result = {};

    Result.RenderOffset = RenderOffset;
    Result.Type = SpriteType_Animation;
    Result.AnimationSpecId = AnimationSpecId;

    return Result;
}

void RenderText(game_state* GameState,
                const char* Text,
                v2 Position,
                color Color,
                bool32 Centered,
                int32 Layer);

void SetUpdateFunction(game_state* GameState, world_update_function Function);
void DefineWeapon(game_state* GameState,
                  weapon_id WeaponId,
                  real32 FireCooldown,
                  real32 FireSpeed,
                  real32 Inaccuracy,
                  projectile_sprite_id ProjectileSpriteId,
                  const v2* ShotOffsets,
                  uint32 ShotCount,
                  real32 FireKickback,
                  real32 FireKickbackReturnTime,
                  real32 DelayReturnSpeed,
                  v2 DelayMaxDistance,
                  texture_id TextureId);
void DefineShip(game_state* GameState,
                ship_id ShipId,
                real32 MoveAcceleration_Normal,
                real32 MoveAcceleration_Focus,
                real32 MoveSpeedMax_Normal,
                real32 MoveSpeedMax_Focus,
                v2 BoundsCollisionSize,
                real32 CollisionAreaRadius,
                real32 GrazeCollisionAreaRadius,
                real32 FatalCollisionTime,
                int32 BombMeterMax,
                int32 BombCountMax,
                int32 BombDuration,
                real32 BombGrowthRate,
                v2 RenderOffset,
                const sprite_spec* Sprites,
                uint32 SpriteCount,
                const sprite_spec* FocusSprites,
                uint32 FocusSpriteCount);
void SetPlayerShipId(game_state* GameState, ship_id Ship_Id);
void SetPlayerWeaponId_Normal(game_state* GameState, weapon_id WeaponId);
void SetPlayerWeaponId_Focus(game_state* GameState, weapon_id WeaponId);
void LoadTexture(game_state* GameState,
                 texture_id TextureId,
                 const char* File,
                 texture_filter TextureFilter);
void AddAnimationSpec(game_state* GameState,
                      animation_spec_id AnimationSpecId,
                      texture_id TextureId,
                      int32 FrameTime);
void AddAnimationFrame(game_state* GameState,
                       animation_spec_id AnimationSpecId,
                       rect Rect);
void LoadScrollingBackground(game_state* GameState,
                             texture_id TextureId,
                             int32 RepeatY,
                             real32 ScrollSpeed);
void LoadSound(game_state* GameState, sound_id SoundId, const char* Path);
void PlaySound(game_state* GameState, sound_id SoundId, real32 Volume);

void DefineBulletSprite(game_state* GameState,
                        projectile_sprite_id SpecId,
                        rect SourceRect,
                        real32 Radius,
                        projectile_directionality Directionality,
                        color_tint DefaultTintMode,
                        blend_mode DefaultBlendMode);
v2 GetPosition(projectile* Projectile);
void SetPosition(projectile* Projectile, v2 Position);
real32 GetAngle(projectile* Projectile);
void SetAngle(projectile* Projectile, real32 Angle);
v2 GetVelocity(projectile* Projectile);
void SetVelocity(projectile* Projectile, v2 Velocity);
v2 GetAcceleration(projectile* Projectile);
void SetAcceleration(projectile* Projectile, real32 Angle, real32 AccelerationMagnitude, real32 SpeedMaxMagnitude);
uint32 GetSpriteId(projectile* Projectile);
void SetSpriteId(projectile* Projectile, uint32 SpriteId);
real32 GetStoredVariable(projectile* Projectile, uint32 Number);
void SetStoredVariable(projectile* Projectile, uint32 Number, real32 Value);
projectile_type GetType(projectile* Projectile);
void DestroyProjectile(game_state* GameState, projectile* Projectile);
void Projectile_Fade(game_state* GameState, projectile* Projectile);
void Projectile_StartShake(projectile* Projectile, real32 Magnitude);
void Projectile_SetAlpha(projectile* Projectile, real32 Alpha);

void StraightLaser_SetAngle(projectile* Projectile, real32 Angle);
real32 StraightLaser_GetAngle(projectile* Projectile);

v2 GetPosition(boss* Boss);
void SetPosition(boss* Boss, v2 Position);
bool32 IsActive(boss* Boss);
real32 GetStoredVariable(boss* Boss, uint32 Number);
void SetStoredVariable(boss* Boss, uint32 Number, real32 Value);
boss* CreateBoss(game_state* GameState, real32 DamageRadius, real32 PlayerKillRadius);
void AddBossAttack(boss* Boss, boss_behavior_function Behavior, int32 Health, int32 Duration);
boss* GetBoss(game_state* GameState);

real32 GetAngleToPlayer(game_state* GameState, v2 Position);
v2 GetPlayerPosition(game_state* GameState);
uint32 GetActiveProjectileCount(game_state* GameState);
projectile* GetProjectileById(game_state* GameState, uint32 Id);
void GetActiveProjectilesInCircle(game_state* GameState, v2 Position, real32 Radius,
                                  uint32* ProjectileIds,
                                  uint32* Count);

void SetBulletBounds(game_state* GameState, real32 Bounds);

projectile*
CreateBullet(game_state* GameState,
             v2 Position,
             v2 Velocity,
             v2 Acceleration,
             v2 SpeedMax,
             int32 SpawnDelay,
             int32 SpriteId,
             color Color,
             int32 Layer,
             projectile_update_function UpdateFunction,
             int32 Damage,
             target Target);

projectile*
CreateBullet_Angle(game_state* GameState,
                   v2 Position,
                   real32 Angle,
                   real32 Speed,
                   real32 AccelerationMagnitude,
                   real32 SpeedMaxMagnitude,
                   int32 SpawnDelay,
                   int32 SpriteId,
                   color Color,
                   int32 Layer,
                   projectile_update_function UpdateFunction,
                   int32 Damage,
                   target Target);


projectile*
CreateLooseLaser(game_state* GameState,
                 v2 Position,
                 real32 Angle,
                 real32 Speed,
                 real32 Width,
                 real32 Length,
                 int32 SpawnDelay,
                 int32 SpriteId,
                 color Color,
                 int32 Layer,
                 projectile_update_function UpdateFunction);

projectile*
CreateStraightLaser(game_state* GameState,
                    v2 Position,
                    real32 Angle,
                    real32 Length,
                    real32 Width,
                    int32 TimeToLive,
                    int32 SpawnDelay,
                    int32 SpriteId,
                    color Color,
                    int32 Layer,
                    projectile_update_function UpdateFunction);

#define Event_BeginTimer(StartTime) \
    int32 Timer = StartTime; \
    int32 Event_LoopIteration = 0; \
    int32 Event_LoopIterationMax = 0; \
    real32 Event_LoopIterationPercent = 0;

#define Event_BeginRepeatingTimer(StartTime, RepeatingInterval) \
    Event_BeginTimer((StartTime) % (RepeatingInterval));

#define Event_At(EventTime) \
    if (Timer == (EventTime))

#define Event_FromTo(EventBegin, EventEnd, EventStep) \
    Event_LoopIteration = FloorReal32((real32)(Timer - (EventBegin)) / (real32)(EventStep)); \
    Event_LoopIterationMax = FloorReal32((real32)(EventEnd - EventBegin) / (real32)(EventStep)); \
    Event_LoopIterationPercent = (real32)Event_LoopIteration / (real32)Event_LoopIterationMax; \
    if (Timer >= (EventBegin) && Timer <= (EventEnd) && !((Timer - (EventBegin)) % (EventStep)))

#define Event_Loop(EventBegin, Loops, EventStep) \
    Event_LoopIteration = FloorReal32((real32)(Timer - (EventBegin)) / (real32)(EventStep)); \
    Event_LoopIterationMax = Loops; \
    Event_LoopIterationPercent = (real32)Event_LoopIteration / (real32)Event_LoopIterationMax; \
    if (Timer >= (EventBegin) && Event_LoopIteration <= Loops && !((Timer - (EventBegin)) % (EventStep)))

#define Event_MoveTo(Object, TargetPosition, Time) \
    (Object)->Spatial.Position = Lerp((Object)->Spatial.Position, (TargetPosition), Time); \

#define Event_MoveTo_QuadIn(Object, OriginalPosition, TargetPosition, Time) \
    SetPosition((Object), Lerp((OriginalPosition), (TargetPosition), Square(Time))); \

#define Event_MoveTo_QuadOut(Object, OriginalPosition, TargetPosition, Time) \
    SetPosition((Object), Lerp((TargetPosition), (OriginalPosition), Square(1.0f - Time))); \

#define For(VariableName, Loops) \
    for(int32 VariableName = 0; \
        VariableName < Loops; \
        ++VariableName)

#define TimeToFrames(Seconds) \
    FloorReal32(Seconds * (real32)GameUpdateHz)

#define FramesToTime(Frames) \
    CeilReal32(Frames / (real32)GameUpdateHz)

enum array_type
{
    ArrayType_uint32,
};
void* CreateTemporaryArray(game_state* GameState, uint32 Size, array_type ArrayType);

#define CreateArray(GameState, Size, Type) \
    (Type*)CreateTemporaryArray(GameState, Size, ArrayType_##Type);
