#pragma once

#include "game.h"
#include "entity.h"

struct particle
{
    int32 TimeToLive;

    sprite Sprite;

    real32 StoredVariables[ProjectileStorageSize];
    particle_update_function UpdateFunction;

    // Transient
    bool32 Active;
    int32 Timer;
    spatial Spatial;
    real32 Scale;
    color_spec ColorSpec;
    blend_mode BlendMode;
    int32 Layer;
    v3 Rotation;

    interpolator ScaleInterpolator;
    interpolator AlphaInterpolator;
};

// Note: Only called by other CreateParticle functions!
particle*
CreateParticleCommon(game_mode_world* World,
                     v2 Position,
                     v2 Velocity,
                     int32 TimeToLive,
                     real32 Scale,
                     color_spec ColorSpec,
                     blend_mode BlendMode,
                     int32 Layer,
                     particle_update_function UpdateFunction)
{
    Assert("Out of particle memory.", World->ParticleCount < ParticleCountMax);

    particle* Result = 0;
    for (uint32 ParticleIndex = 0;
         ParticleIndex < ParticleCountMax;
         ++ParticleIndex)
    {
        particle* Particle= World->Particles + ParticleIndex;
        if (!Particle->Active)
        {
            Particle->TimeToLive = TimeToLive;
            Particle->Scale = Scale;
            Particle->ScaleInterpolator.Active = false;
            Particle->ScaleInterpolator.ValuePointer = &Particle->Scale;
            Particle->ColorSpec = ColorSpec;
            Particle->BlendMode = BlendMode;
            Particle->AlphaInterpolator.Active = false;
            Particle->AlphaInterpolator.ValuePointer = &Particle->ColorSpec.Color.A;
            Particle->Rotation = V3(0.0f, 0.0f, 0.0f);
            Particle->Layer = Layer;
            Particle->UpdateFunction = UpdateFunction;

            Particle->Active = true;
            Particle->Timer = 0;
            InitializeSpatial_Linear(&Particle->Spatial, Position, Velocity);

            ++World->ParticleCount;
            Result = Particle;
            break;
        }
    }

    Assert("No ununsed particles in pool.", Result);
    return Result;
}

particle*
CreateParticle(game_state* GameState,
               v2 Position,
               v2 Velocity,
               int32 TimeToLive,
               real32 Scale,
               color_spec ColorSpec,
               blend_mode BlendMode,
               int32 Layer,
               texture_id TextureId,
               particle_update_function UpdateFunction)
{
    game_mode_world* World = GameState->World;
    particle* Particle = CreateParticleCommon(World, Position, Velocity, TimeToLive,
                                              Scale, ColorSpec, BlendMode, Layer, UpdateFunction);
    InitializeSprite(&Particle->Sprite, TextureId);

    return Particle;
}

particle*
CreateParticle_Animated(game_state* GameState,
                        v2 Position,
                        v2 Velocity,
                        int32 TimeToLive,
                        real32 Scale,
                        color_spec ColorSpec,
                        blend_mode BlendMode,
                        int32 Layer,
                        animation_spec_id AnimationSpecId,
                        particle_update_function UpdateFunction)
{
    game_mode_world* World = GameState->World;
    particle* Particle = CreateParticleCommon(World, Position, Velocity, TimeToLive,
                                              Scale, ColorSpec, BlendMode, Layer, UpdateFunction);
    InitializeSprite_Animated(&Particle->Sprite, AnimationSpecId);
    return Particle;
}

void
CreateExplosion(game_state* GameState,
                v2 Position,
                real32 SpawnRadius,
                real32 ParticleSpeedMin,
                real32 ParticleSpeedMax,
                int32 ParticleTimeToLiveMin,
                int32 ParticleTimeToLiveMax,
                texture_id TextureId,
                uint32 ParticleCount)
{
    Assert("Passed Max < Min", ParticleSpeedMax >= ParticleSpeedMin);
    Assert("Passed Max < Min", ParticleTimeToLiveMax >= ParticleTimeToLiveMin);

    real32 ParticleSpeedRange = ParticleSpeedMax - ParticleSpeedMin;
    int32 ParticleTimeToLiveRange = ParticleTimeToLiveMax - ParticleTimeToLiveMin;

    game_mode_world* World = GameState->World;
    for (uint32 ParticleIndex = 0;
         ParticleIndex < ParticleCount;
         ++ParticleIndex)
    {
        v2 PositionOffset = V2(RandomPerturb(GameState, 0.0f, SpawnRadius),
                               RandomPerturb(GameState, 0.0f, SpawnRadius));
        v2 ParticleDirection = GetDirectionFromAngle(RandomRange(GameState, 360.0f));
        real32 ParticleSpeed = RandomRange(GameState, ParticleSpeedMin, ParticleSpeedRange);
        v2 ParticleVelocity = ParticleDirection * ParticleSpeed;
        int32 ParticleTimeToLive = RoundReal32(RandomRange(GameState, ParticleTimeToLiveMin, ParticleTimeToLiveRange));
        particle* Particle = CreateParticle(GameState, Position + PositionOffset,
                                            ParticleVelocity, ParticleTimeToLive, 0.5f,
                                            ColorSpec(Color(Colors.White, 0.8f), ColorTint_Mod),
                                            BlendMode_Blend,
                                            1, TextureId, 0);
        Interpolate(&Particle->AlphaInterpolator, 0.6f, 0.0f, 30, ParticleTimeToLive);
    }

    CreateParticle_Animated(GameState, Position, V2(0.0f, 0.0f),
                            15, 1.0f,
                            ColorSpec_None, BlendMode_Blend,
                            20, AnimationSpecId_Explosion_0, 0);
}

void
CreateExplosion2(game_state* GameState,
                 v2 Position)
{
    CreateParticle_Animated(GameState,
                            Position,
                            V2(0.0f, 0.0f),
                            15, 1.0f,
                            ColorSpec_None, BlendMode_Blend,
                            20, AnimationSpecId_Explosion_1, 0);
}

void
CreateGrazeParticle(game_state* GameState,
                    v2 GrazePosition)
{
    game_mode_world* World = GameState->World;
    real32 ParticleSpeedMin = 0.3f;
    real32 ParticleSpeedRange = 0.85f;
    v2 ParticleVelocity = V2(RandomSign(GameState) * RandomRange(GameState, ParticleSpeedMin, ParticleSpeedRange),
                             RandomSign(GameState) * RandomRange(GameState, ParticleSpeedMin, ParticleSpeedRange));
    real32 ParticleTimeToLive = 40;
    particle* Particle = CreateParticle(GameState,
                                        GrazePosition,
                                        ParticleVelocity,
                                        ParticleTimeToLive,
                                        0.5f,
                                        ColorSpec_None,
                                        BlendMode_Blend,
                                        1, TextureId_Graze, 0);
    Interpolate(&Particle->ScaleInterpolator, Particle->Scale, 0.3f, 0, ParticleTimeToLive);
    Interpolate(&Particle->AlphaInterpolator, Particle->ColorSpec.Color.A, 0.0f, 30, ParticleTimeToLive);
}

void
UpdateParticles(game_state* GameState,
                real32 DeltaTime)
{
    game_mode_world* World = GameState->World;
    for (uint32 ParticleIndex = 0;
         ParticleIndex < ParticleCountMax;
         ++ParticleIndex)
    {
        particle* Particle = World->Particles + ParticleIndex;
        if (Particle->Active)
        {
            if (Particle->UpdateFunction)
            {
                Particle->UpdateFunction(GameState, Particle, Particle->Timer);
                if (!Particle->Active)
                {
                    continue;
                }
            }

            UpdateSpatial(&Particle->Spatial);

            ++Particle->Timer;
            if (Particle->Timer > Particle->TimeToLive)
            {
                Particle->Active = false;
                --World->ParticleCount;
                continue;
            }

            // TODO: Keep these in a list
            UpdateInterpolator(&Particle->ScaleInterpolator, Particle->Timer);
            UpdateInterpolator(&Particle->AlphaInterpolator, Particle->Timer);

            // Render
            UpdateSprite(GameState, &Particle->Sprite);
            PushSprite(GameState,
                       &Particle->Sprite,
                       Particle->Spatial.Position,
                       Particle->Scale,
                       V2(0.0f, 0.0f), Particle->Rotation,
                       Particle->ColorSpec,
                       Particle->Layer, Particle->BlendMode);
        }
    }
}

void
ClearParticles(game_mode_world* World)
{
    World->ParticleCount = 0;
    for (uint32 ParticleIndex = 0;
         ParticleIndex < ParticleCountMax;
         ++ParticleIndex)
    {
        particle* Particle = World->Particles + ParticleIndex;
        Particle->Active = false;
    }
}

// API
void
Particle_SetPosition(particle* Particle, v2 Position)
{
    Particle->Spatial.Position = Position;
}

void
Particle_ChangePosition(particle* Particle, v2 DeltaPosition)
{
    Particle->Spatial.Position += DeltaPosition;
}

void
Particle_SetAlpha(particle* Particle, real32 Alpha)
{
    Particle->ColorSpec.Color.A = Alpha;
}

real32
Particle_GetScale(particle* Particle)
{
    return Particle->Scale;
}

void
Particle_SetScale(particle* Particle, real32 Scale)
{
    Particle->Scale = Scale;
}

void
Particle_SetRotation(particle* Particle, v3 Rotation)
{
    Particle->Rotation = Rotation;
}

void
Particle_ChangeRotation(particle* Particle, v3 DeltaRotation)
{
    Particle->Rotation += DeltaRotation;
}

int32
Particle_GetTimeToLive(particle* Particle)
{
    return Particle->TimeToLive;
}

real32
GetStoredVariable(particle* Particle, uint32 Number)
{
    return Particle->StoredVariables[Number];
}

void
SetStoredVariable(particle* Particle, uint32 Number, real32 Value)
{
    Particle->StoredVariables[Number] = Value;
}
