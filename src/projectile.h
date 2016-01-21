#pragma once

#include "world.h"
#include "entity.h"
#include "render.h"
#include "api.h"

struct bullet_sprite
{
    real32 Radius;
    rect TextureRect;
    projectile_directionality Directionality;
    color_tint DefaultTintMode;
    blend_mode DefaultBlendMode;
};

struct bullet
{
    collision_area_circle CollisionArea;

    bool32 Shaking;
    real32 ShakeMagnitude;
    v2 ShakeAnchorPosition;
};

struct straight_laser
{
    real32 Angle;
    real32 Length;
    real32 Width;
    int32 TimeToLive;
};

struct loose_laser
{
    real32 Angle;
    real32 Speed;
    real32 Length;
    real32 LengthMax;
    real32 Width;
};

const int32 ProjectileStorageSize = 8;

struct projectile
{
    bool32 Active;
    bool32 Collidable;
    bool32 Grazed;

    int32 CreationTime;
    target Target;
    int32 Damage;
    spatial Spatial;
    int32 SpawnDelay;

    // NOTE: There must be a better way... but in the meantime it will have to do.
    // Scripting language would fix this.
    real32 StoredVariables[ProjectileStorageSize];
    projectile_update_function UpdateFunction;

    // Render Info
    projectile_sprite_id SpriteId;
    color_spec ColorSpec;
    int32 Layer;

    // Fading
    interpolator AlphaInterpolator;
    bool32 FadingOut;
    int32 FadeOutEndTime;

    projectile_type Type;
    union
    {
        bullet Bullet;
        loose_laser LooseLaser;
        straight_laser StraightLaser;
    };
};

void
InitializeProjectile(projectile* Projectile,
                     projectile_type Type,
                     v2 Position,
                     v2 Velocity,
                     v2 Acceleration,
                     v2 SpeedMax,
                     int32 SpawnDelay,
                     int32 CreationTime,
                     int32 Damage,
                     target Target,
                     projectile_sprite_id SpriteId,
                     color_spec ColorSpec,
                     int32 Layer,
                     projectile_update_function UpdateFunction)
{
    Warn("Projectile created outside of its Render Zone.",
         Layer <= RenderZone_World && Layer > RenderZone_UI);
    Projectile->Active = true;
    Projectile->Collidable = true;
    Projectile->Grazed = false;
    InitializeSpatial(&Projectile->Spatial, Position, Velocity, Acceleration, SpeedMax);
    Projectile->SpawnDelay = SpawnDelay;
    Projectile->CreationTime = CreationTime;
    Projectile->UpdateFunction = UpdateFunction;
    Projectile->Damage = Damage;
    Projectile->Target = Target;
    Projectile->Type = Type;
    Projectile->SpriteId = SpriteId;
    Projectile->ColorSpec = ColorSpec;
    Projectile->Layer = Layer;
    Projectile->AlphaInterpolator.Active = false;
    Projectile->AlphaInterpolator.ValuePointer = &Projectile->ColorSpec.Color.A;
    Projectile->FadingOut = false;
    // TODO: Initialize projectile's storage to 0?
}

int32
Projectile_GetSpriteId(projectile* Projectile)
{
    return Projectile->SpriteId;
}

void
DestroyProjectile(game_state* GameState, projectile* Projectile)
{
    game_mode_world* World = GameState->World;
    Assert("No projectiles to destroy.", World->ProjectileCount > 0);
    Projectile->Active = false;
    --World->ProjectileCount;
}

projectile*
GetInactiveProjectile(game_mode_world* World)
{
    projectile* Found = 0;

    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        if (!Projectile->Active)
        {
            Found = Projectile;
        }
    };

    Assert("Out of projectile memory.", Found);
    return Found;
}

projectile*
CreateStraightLaser(game_state* GameState,
                    v2 Position,
                    // real32 Speed,
                    // real32 Acceleration,
                    // real32 SpeedMax,
                    real32 Angle,
                    real32 Length,
                    real32 Width,
                    int32 TimeToLive,
                    int32 SpawnDelay,
                    int32 SpriteId,
                    color Color,
                    int32 Layer,
                    projectile_update_function UpdateFunction)
{
    game_mode_world* World = GameState->World;
    projectile* NewProjectile = GetInactiveProjectile(World);
    if (NewProjectile)
    {
        bullet_sprite* BulletSprite = &GameState->Assets.BulletSprites[SpriteId];
        InitializeProjectile(NewProjectile,
                             Projectile_StraightLaser,
                             Position,
                             V2(0.0f, 0.0f),
                             V2(0.0f, 0.0f),
                             V2(0.0f, 0.0f),
                             SpawnDelay,
                             World->Timer,
                             1, Target_Player,
                             (projectile_sprite_id)SpriteId,
                             ColorSpec(Color, BulletSprite->DefaultTintMode),
                             Layer,
                             UpdateFunction);

        straight_laser* StraightLaser = &NewProjectile->StraightLaser;
        StraightLaser->Angle = Angle;
        StraightLaser->Length = Length;
        StraightLaser->Width = Width;
        StraightLaser->TimeToLive = TimeToLive;
    }

    ++World->ProjectileCount;
    return NewProjectile;
}

void
Projectile_Fade(game_state* GameState,
                projectile* Projectile)
{
    int32 Time = GameState->World->Timer;
    int32 EndTime = Time + ProjectileFadeTime;
    Projectile->Collidable = false;
    Projectile->FadingOut = true;
    Projectile->FadeOutEndTime = EndTime;
    Interpolate(&Projectile->AlphaInterpolator, Projectile->ColorSpec.Color.A, 0.0f, Time, EndTime);
}

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
                 projectile_update_function UpdateFunction)
{
    game_mode_world* World = GameState->World;
    projectile* NewProjectile = GetInactiveProjectile(World);
    if (NewProjectile)
    {
        bullet_sprite* BulletSprite = &GameState->Assets.BulletSprites[SpriteId];
        InitializeProjectile(NewProjectile,
                             Projectile_LooseLaser,
                             Position,
                             V2(0.0f, 0.0f),
                             V2(0.0f, 0.0f),
                             V2(0.0f, 0.0f),
                             SpawnDelay,
                             World->Timer,
                             1, Target_Player,
                             (projectile_sprite_id)SpriteId,
                             ColorSpec(Color, BulletSprite->DefaultTintMode),
                             Layer,
                             UpdateFunction);

        loose_laser* LooseLaser = &NewProjectile->LooseLaser;
        LooseLaser->Angle = Angle;
        LooseLaser->Speed = Speed;
        LooseLaser->Length = 0.0f;
        LooseLaser->Width = Width;
        LooseLaser->LengthMax = Length;
    }

    ++World->ProjectileCount;
    return NewProjectile;
}

inline v2 GetPlayerPosition(game_state* GameState);

void
DefineBulletSprite(game_state* GameState,
                   projectile_sprite_id SpecId,
                   rect SourceRect,
                   real32 Radius,
                   projectile_directionality Directionality,
                   color_tint DefaultTintMode,
                   blend_mode DefaultBlendMode)
{
    bullet_sprite* BulletSprite = GameState->Assets.BulletSprites + SpecId;
    BulletSprite->TextureRect = SourceRect;
    BulletSprite->Radius = Radius;
    BulletSprite->Directionality = Directionality;
    BulletSprite->DefaultTintMode = DefaultTintMode;
    BulletSprite->DefaultBlendMode = DefaultBlendMode;
}

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
             target Target)
{
    game_mode_world* World = GameState->World;
    projectile* NewProjectile = GetInactiveProjectile(World);
    if (NewProjectile)
    {
        bullet_sprite* BulletSprite = &GameState->Assets.BulletSprites[SpriteId];
        InitializeProjectile(NewProjectile,
                             Projectile_Bullet,
                             Position,
                             Velocity,
                             Acceleration,
                             SpeedMax,
                             SpawnDelay,
                             World->Timer,
                             1, Target,
                             (projectile_sprite_id)SpriteId,
                             ColorSpec(Color, BulletSprite->DefaultTintMode),
                             Layer,
                             UpdateFunction);

        bullet* Bullet = &NewProjectile->Bullet;
        InitializeCollisionArea_Circle(&Bullet->CollisionArea, BulletSprite->Radius);
        Bullet->Shaking = false;
    }

    ++World->ProjectileCount;
    return NewProjectile;
}

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
                   target Target)
{
    v2 Velocity = PolarToCartesian(Angle, Speed);
    v2 Acceleration = PolarToCartesian(Angle, AccelerationMagnitude);
    v2 SpeedMax = PolarToCartesian(Angle, SpeedMaxMagnitude);
    return CreateBullet(GameState,
                        Position,
                        Velocity,
                        Acceleration,
                        SpeedMax,
                        SpawnDelay,
                        SpriteId,
                        Color,
                        Layer,
                        UpdateFunction, Damage, Target);
}

v2
GetVelocity(projectile* Projectile)
{
    return Projectile->Spatial.Velocity;
}

void
SetVelocity(projectile* Projectile, v2 Velocity)
{
    Projectile->Spatial.Velocity = Velocity;
}

real32
GetAngle(projectile* Projectile)
{
    return VectorAngle(Projectile->Spatial.Velocity);
}

void
SetAngle(projectile* Projectile, real32 Angle)
{
    real32 Speed = Magnitude(Projectile->Spatial.Velocity);
    v2 NewDirection = GetDirectionFromAngle(Angle);
    Projectile->Spatial.Velocity = Rescale(NewDirection, Speed);
}

v2
GetAcceleration(projectile* Projectile)
{
    return Projectile->Spatial.Acceleration;
}

void
SetAcceleration(projectile* Projectile,
                real32 Angle,
                real32 AccelerationMagnitude,
                real32 SpeedMaxMagnitude)
{
    v2 Direction = GetDirectionFromAngle(Angle);
    Projectile->Spatial.Acceleration = Rescale(Direction, AccelerationMagnitude);
    Projectile->Spatial.SpeedMax = Rescale(Direction, SpeedMaxMagnitude);
}

bool32
IsOutOfBounds(projectile* Projectile,
              real32 Bounds)
{
    bool32 Result = false;

    v2 Position = Projectile->Spatial.Position;
    if (Position.X > WorldSizeX + Bounds ||
        Position.X < -Bounds ||
        Position.Y > WorldSizeY + Bounds ||
        Position.Y < -Bounds)
    {
        Result = true;
    }

    return Result;
}

void
UpdateProjectiles(game_state* GameState,
                  int32 Time, real32 Bounds)
{
    game_mode_world* World = GameState->World;
    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        bullet_sprite* BulletSprite = &GameState->Assets.BulletSprites[Projectile->SpriteId];
        if (Projectile->Active)
        {
            int32 ProjectileElapsed = Time - Projectile->CreationTime;
            if (Projectile->UpdateFunction)
            {
                Projectile->UpdateFunction(GameState, Projectile, ProjectileElapsed - Projectile->SpawnDelay);
                if (!Projectile->Active)
                {
                    continue;
                }
            }

            if (Projectile->FadingOut && Projectile->FadeOutEndTime == Time)
            {
                DestroyProjectile(GameState, Projectile);
                continue;
            }

            switch(Projectile->Type)
            {
                case Projectile_Bullet:
                {
                    bullet* Bullet = &Projectile->Bullet;
                    if (ProjectileElapsed < Projectile->SpawnDelay)
                    {
                        bullet_sprite* DelaySprite = &GameState->Assets.BulletSprites[ProjectileSprite_Delay];
                        v2 Center = V2(DelaySprite->TextureRect.W / 2.0f,
                                       DelaySprite->TextureRect.H / 2.0f);
                        real32 DelayAlpha = 1.0f - ((real32)ProjectileElapsed / (real32)Projectile->SpawnDelay);
                        PushTextureEx(GameState->Renderer,
                                      TextureId_BulletMap,
                                      DelaySprite->TextureRect,
                                      Projectile->Spatial.Position, 1.0f,
                                      Center, 0.0f,
                                      ColorSpec(Color(Colors.White, DelayAlpha), ColorTint_Luminosity),
                                      Projectile->Layer, BulletSprite->DefaultBlendMode);
                    }
                    else
                    {
                        UpdateInterpolator(&Projectile->AlphaInterpolator, Time);
                        UpdateSpatialVelocity(&Projectile->Spatial);

                        if (Bullet->Shaking)
                        {
                            // Update Position
                            Bullet->ShakeAnchorPosition += Projectile->Spatial.Velocity;
                            v2 ShakeMagnitude = V2(RandomRange(GameState, Bullet->ShakeMagnitude), RandomRange(GameState, Bullet->ShakeMagnitude));
                            Projectile->Spatial.Position = Bullet->ShakeAnchorPosition + ShakeMagnitude;
                        }
                        else
                        {
                            UpdateSpatialPosition(&Projectile->Spatial);
                        }
                        if (IsOutOfBounds(Projectile, Bounds))
                        {
                            DestroyProjectile(GameState, Projectile);
                            break;
                        }

                        // Render
                        // Bullet sprites are assumed to be pointing up, so we add 90 degrees to align it with (1,0).
                        real32 Angle = 0.0f;
                        if (BulletSprite->Directionality == Directionality_Angle)
                        {
                            Angle = 90.0f + VectorAngle(Projectile->Spatial.Velocity);
                        }
                        v2 Center = V2(BulletSprite->TextureRect.W / 2.0f,
                                       BulletSprite->TextureRect.H / 2.0f);
                        PushTextureEx(GameState->Renderer,
                                      TextureId_BulletMap,
                                      BulletSprite->TextureRect,
                                      Projectile->Spatial.Position, 1.0f,
                                      Center, Angle,
                                      Projectile->ColorSpec,
                                      Projectile->Layer, BulletSprite->DefaultBlendMode);
                    }

                } break;
                case Projectile_LooseLaser:
                {
                    loose_laser* LooseLaser = &Projectile->LooseLaser;
                    if (ProjectileElapsed < Projectile->SpawnDelay)
                    {
                        // Render pre-spawn glow
                    }
                    else
                    {
                        UpdateInterpolator(&Projectile->AlphaInterpolator, Time);

                        LooseLaser->Length += LooseLaser->Speed;
                        if (LooseLaser->Length >= LooseLaser->LengthMax)
                        {
                            LooseLaser->Length = LooseLaser->LengthMax;
                            v2 Velocity = GetDirectionFromAngle(LooseLaser->Angle) * LooseLaser->Speed;
                            Projectile->Spatial.Position += Velocity;
                        }

                        // TODO: Unify
                        v2 Position = Projectile->Spatial.Position;
                        if (Position.X < 0 ||
                            Position.Y < 0 ||
                            Position.X > WorldSizeX ||
                            Position.Y > WorldSizeY)
                        {
                            DestroyProjectile(GameState, Projectile);
                            continue;
                        }

                        // Render
                        rect SourceRect = GameState->Assets.BulletSprites[Projectile->SpriteId].TextureRect;
                        rect DestRect = Rect(
                            Position.X - (LooseLaser->Width / 2.0f),
                            Position.Y,
                            LooseLaser->Width,
                            LooseLaser->Length);
                        v2 Center = V2(LooseLaser->Width / 2.0f, 0.0f);
                        PushTextureEx(GameState->Renderer,
                                      TextureId_BulletMap,
                                      SourceRect,
                                      DestRect,
                                      Center,
                                      LooseLaser->Angle - 90,
                                      Projectile->ColorSpec,
                                      Projectile->Layer,
                                      BulletSprite->DefaultBlendMode);
                    }
                } break;
                case Projectile_StraightLaser:
                {
                    straight_laser* StraightLaser = &Projectile->StraightLaser;
                    real32 Width;
                    if (ProjectileElapsed < Projectile->SpawnDelay)
                    {
                        Projectile->Collidable = false;
                        Width = 2.0f;
                    }
                    else
                    {
                        if (ProjectileElapsed == Projectile->SpawnDelay)
                        {
                            Projectile->Collidable = true;
                        }

                        Width = StraightLaser->Width;
                        UpdateInterpolator(&Projectile->AlphaInterpolator, Time);
                    }

                    v2 Position = Projectile->Spatial.Position;
                    rect SourceRect = BulletSprite->TextureRect;
                    rect DestRect = Rect(Position.X - (Width / 2.0f),
                                         Position.Y,
                                         Width,
                                         StraightLaser->Length);
                    v2 Center = V2(RoundReal32(Width / 2.0f), 0.0f);
                    PushTextureEx(GameState->Renderer,
                                  TextureId_BulletMap,
                                  SourceRect,
                                  DestRect,
                                  Center,
                                  StraightLaser->Angle - 90,
                                  Projectile->ColorSpec,
                                  Projectile->Layer, BulletSprite->DefaultBlendMode);

                    bullet_sprite* GlowSprite = &GameState->Assets.BulletSprites[ProjectileSprite_Delay];
                    v2 GlowCenter = V2(GlowSprite->TextureRect.W / 2.0f,
                                       GlowSprite->TextureRect.H / 2.0f);
                    PushTextureEx(GameState->Renderer,
                                  TextureId_BulletMap,
                                  GlowSprite->TextureRect,
                                  Projectile->Spatial.Position, 0.5f,
                                  GlowCenter, 0.0f,
                                  Projectile->ColorSpec,
                                  Projectile->Layer, BulletSprite->DefaultBlendMode);

                    if (Projectile->Collidable && ProjectileElapsed - Projectile->SpawnDelay > StraightLaser->TimeToLive)
                    {
                        Projectile_Fade(GameState, Projectile);
                    }
                } break;
            }
        }
    }
}

void
FadeProjectiles_Targeted(game_state* GameState, target Target)
{
    game_mode_world* World = GameState->World;
    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        if (Projectile->Active && Projectile->Target == Target)
        {
            Projectile_Fade(GameState, Projectile);
        }
    }
}

void
ClearProjectiles_Targeted(game_state* GameState, target Target)
{
    game_mode_world* World = GameState->World;
    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        if (Projectile->Active && Projectile->Target == Target)
        {
            DestroyProjectile(GameState, Projectile);
        }
    }
}

void
ClearProjectiles(game_state* GameState)
{
    game_mode_world* World = GameState->World;

    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        Projectile->Active = false;
    }

    World->ProjectileCount = 0;
}

/////////
// API //
/////////

v2
GetPosition(projectile* Projectile)
{
    return Projectile->Spatial.Position;
}

void
SetPosition(projectile* Projectile, v2 Position)
{
    Projectile->Spatial.Position = Position;
}

real32
GetStoredVariable(projectile* Projectile, uint32 Index)
{
    Assert("Variable slot out of bounds.", Index < ProjectileStorageSize);
    return Projectile->StoredVariables[Index];
}

void
SetStoredVariable(projectile* Projectile, uint32 Index, real32 Value)
{
    Assert("Variable slot out of bounds.", Index < ProjectileStorageSize);
    Projectile->StoredVariables[Index] = Value;
}

void
Projectile_StartShake(projectile* Projectile, real32 Magnitude)
{
    Assert("Must be a bullet for now.\n", Projectile->Type == Projectile_Bullet);
    Projectile->Bullet.Shaking = true;
    Projectile->Bullet.ShakeMagnitude = Magnitude;
    Projectile->Bullet.ShakeAnchorPosition = Projectile->Spatial.Position;
}

void
Projectile_SetAlpha(projectile* Projectile, real32 Alpha)
{
    Assert("Projectile must be a straight laser.", Projectile->Type == Projectile_StraightLaser);
    Projectile->ColorSpec.Color.A = Alpha;
}

uint32
GetActiveProjectileCount(game_state* GameState)
{
    game_mode_world* World = GameState->World;
    return World->ProjectileCount;
}

projectile*
GetProjectileById(game_state* GameState, uint32 Id)
{
    game_mode_world* World = GameState->World;
    return &World->Projectiles[Id];
}

void
GetActiveProjectilesInCircle(game_state* GameState,
                             v2 Position, real32 Radius,
                             uint32* ProjectileIds, // Out
                             uint32* Count) // Out
{
    game_mode_world* World = GameState->World;
    uint32 ResultCount = 0;

    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        v2 ProjectilePosition = Projectile->Spatial.Position;
        bool32 InRadius = false;
        switch (Projectile->Type)
        {
            case Projectile_Bullet:
            {
                v2 ProjectileToPosition = Position - ProjectilePosition;
                v2 ClosestPoint = ProjectilePosition + Rescale(ProjectileToPosition, Projectile->Bullet.CollisionArea.Radius);
                InRadius = Distance(ClosestPoint, Position) < Radius;
            } break;
            case Projectile_LooseLaser:
            {
                InRadius = Distance(Projectile->Spatial.Position, Position) < Radius;
            } break;
            case Projectile_StraightLaser:
            {
                InRadius = Distance(Projectile->Spatial.Position, Position) < Radius;
            } break;
        }

        if (Projectile->Active &&
            !Projectile->FadingOut &&
            InRadius)
        {
            ProjectileIds[ResultCount] = ProjectileIndex;
            ++ResultCount;
        }
    }

    *Count = ResultCount;
}

uint32
GetSpriteId(projectile* Projectile)
{
    return Projectile->SpriteId;
}

projectile_type
GetType(projectile* Projectile)
{
    return Projectile->Type;
}

void
SetSpriteId(projectile* Projectile, uint32 SpriteId)
{
    Projectile->SpriteId = (projectile_sprite_id)SpriteId;
}

real32
StraightLaser_GetAngle(projectile* Projectile)
{
    Assert("Projectile must be a straight laser.", Projectile->Type == Projectile_StraightLaser);
    return Projectile->StraightLaser.Angle;
}

void
StraightLaser_SetAngle(projectile* Projectile, real32 Angle)
{
    Assert("Projectile must be a straight laser.", Projectile->Type == Projectile_StraightLaser);
    Projectile->StraightLaser.Angle = Angle;
}
