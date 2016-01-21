#pragma once

#include "api.h"

void*
CreateTemporaryArray(game_state* GameState, uint32 Size, array_type ArrayType)
{
    void* Result = 0;

    switch(ArrayType)
    {
        case ArrayType_uint32:
            Result = PushArray(&GameState->TransientState->Memory, Size, uint32);
            break;
    }

    return Result;
}

projectile*
CreateBullet_Linear(game_state* GameState,
                    v2 Position,
                    v2 Velocity,
                    int32 SpawnDelay,
                    int32 SpriteId,
                    color Color,
                    int32 Layer,
                    projectile_update_function UpdateFunction,
                    int32 Damage,
                    target Target)
{
    return CreateBullet(GameState,
                        Position,
                        Velocity,
                        V2(0.0f, 0.0f),
                        V2(0.0f, 0.0f),
                        SpawnDelay,
                        SpriteId,
                        Color,
                        Layer,
                        UpdateFunction,
                        Damage,
                        Target);
}

projectile*
CreateBullet_AngleLinear(game_state* GameState,
                         v2 Position,
                         real32 Angle,
                         real32 Speed,
                         int32 SpawnDelay,
                         int32 SpriteId,
                         color Color,
                         int32 Layer,
                         projectile_update_function UpdateFunction,
                         int32 Damage,
                         target Target)
{
    return CreateBullet_Angle(GameState,
                              Position,
                              Angle,
                              Speed,
                              0.0f,
                              0.0f,
                              SpawnDelay,
                              SpriteId,
                              Color,
                              Layer,
                              UpdateFunction,
                              Damage, Target);
}

void
CreateBulletArc(game_state* GameState,
                uint32 BulletCount,
                real32 ArcDegrees,
                v2 Position,
                real32 Radius,
                real32 Angle,
                real32 Speed,
                real32 AccelerationMagnitude,
                real32 SpeedMaxMagnitude,
                int32 SpawnDelay,
                int32 SpriteId,
                color Color,
                int32 Damage,
                target Target)
{
    real32 ArcInterval = BulletCount > 1 ?
                         (ArcDegrees / (real32)BulletCount):
                         0;
    real32 FirstArcIndex = -((real32)BulletCount - 1.0f) / 2.0f;
    for (uint32 BulletIndex = 0;
         BulletIndex < BulletCount;
         ++BulletIndex)
    {
        real32 SpawnAngle = Angle + (ArcInterval * (FirstArcIndex + BulletIndex));
        v2 PositionOffset = V2(Radius * Cos(Angle), Radius * Sin(Angle));
        CreateBullet_Angle(GameState,
                           Position + PositionOffset,
                           SpawnAngle,
                           Speed, AccelerationMagnitude, SpeedMaxMagnitude,
                           SpawnDelay,
                           SpriteId, Color, Layer_ProjectileDefault,
                           0, Damage, Target);
    }
}
