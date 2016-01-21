#pragma once

#include "game.h"
#include "sound.h"

struct interpolator
{
    bool32 Active;
    real32* ValuePointer;

    real32 StartValue;
    real32 EndValue;
    int32 StartTime;
    int32 EndTime;
};

inline void
UpdateInterpolator(interpolator* Interpolator, int32 Time)
{
    if (!Interpolator->Active)
    {
        return;
    }

    if (Time < Interpolator->StartTime)
    {
        *Interpolator->ValuePointer = Interpolator->StartValue;
        return;
    }

    int32 InterpolationTimeRange = Interpolator->EndTime - Interpolator->StartTime;
    if (InterpolationTimeRange <= 0)
    {
        return;
    }

    int32 TimeSinceStart = Time - Interpolator->StartTime;
    real32 LerpTime = (real32)TimeSinceStart / (real32)InterpolationTimeRange;
    ClampHigh(&LerpTime, 1.0f);
    *Interpolator->ValuePointer = Lerp(Interpolator->StartValue,
                                      Interpolator->EndValue,
                                      LerpTime);
}

inline void
Interpolate(interpolator* Interpolator,
            real32 StartValue,
            real32 EndValue,
            int32 StartTime,
            int32 EndTime)
{
    Interpolator->Active = true;
    *Interpolator->ValuePointer = StartValue;

    Interpolator->StartValue = StartValue;
    Interpolator->EndValue = EndValue;
    Interpolator->StartTime = StartTime;
    Interpolator->EndTime = EndTime;
}

enum movement_type
{
    MovementType_Linear,
    MovementType_Accelerated,
};

struct collision_area_circle
{
    bool32 Collidable;

    real32 Radius;
};

struct spatial
{
    v2 Position;
    v2 Velocity;
    v2 Acceleration;
    v2 SpeedMax;

    movement_type MovementType;
};

void
InitializeSpatial(spatial* Spatial, v2 Position, v2 Velocity, v2 Acceleration, v2 SpeedMax)
{
    Spatial->Position = Position;
    Spatial->Velocity = Velocity;
    Spatial->Acceleration = Acceleration;
    Spatial->SpeedMax = SpeedMax;
    Spatial->MovementType = MovementType_Linear;
}

void
InitializeSpatial_Linear(spatial* Spatial, v2 Position, v2 Velocity)
{
    InitializeSpatial(Spatial, Position, Velocity, V2(0.0f, 0.0f), V2(0.0f, 0.0f));
}

void
InitializeCollisionArea_Circle(collision_area_circle* CollisionArea, real32 Radius)
{
    CollisionArea->Collidable = true;
    CollisionArea->Radius = Radius;
}

bool32
Collides(spatial* SpatialA,
         collision_area_circle* CollisionAreaA,
         spatial* SpatialB,
         collision_area_circle* CollisionAreaB)
{
    real32 TotalRadiusSquared = Square(CollisionAreaA->Radius + CollisionAreaB->Radius);
    return DistanceSquared(SpatialA->Position, SpatialB->Position) < TotalRadiusSquared;
}

bool32
CollidesRect(spatial* Spatial,
             collision_area_circle* CollisionArea,
             v2 RectPosition,
             v2 RectDimension,
             real32 Angle,
             v2 ShrinkFactor)
{
    // NOTE: The RectPosition refers to the "Anchor" of the rectangle, defined as (0, height / 2)
    //            _____________________________________
    //           |                                     |
    // Anchor -> +                                     |
    //           |_____________________________________|
    // NOTE: Collision between a rotated rect and a circle is done by
    // reversing the rotation of both the rect and the circle, then checking
    // if the circle is inside the rect.
    v2 SpatialPosition = Spatial->Position;
    v2 RotatedSpatialPosition = RotateAround(SpatialPosition, RectPosition, -Angle);
    real32 Radius = CollisionArea->Radius;

    v2 RectCorner = V2(RectPosition.X, RectPosition.Y - RectDimension.X/2.0f);
    // The shrinking factor assumes lasers are vertically oriented, so we need to invert the dimensions
    v2 RectDimensionInverted = V2(RectDimension.Y, RectDimension.X);
    // Apply shrinking factor
    v2 RotatedCollisionFactor = V2(ShrinkFactor.Y, ShrinkFactor.X);
    v2 InverseCollisionFactor = V2(1.0f, 1.0f) - RotatedCollisionFactor;
    v2 HalfInverseCollisionFactor = 0.5f * InverseCollisionFactor;
    v2 Adjustment = Hadamard(HalfInverseCollisionFactor, RectDimensionInverted);
    v2 AdjustedRectCorner = RectCorner + Adjustment;
    v2 AdjustedRectDimension = RectDimensionInverted - 2.0f * Adjustment;
    // Add radius to resulting shape
    v2 MinkowskiCorner = AdjustedRectCorner - V2(Radius, Radius);
    v2 MinkowskiDimension = AdjustedRectDimension + V2(2 * Radius, 2 * Radius);
    if (RotatedSpatialPosition.X < MinkowskiCorner.X ||
        RotatedSpatialPosition.X > MinkowskiCorner.X + MinkowskiDimension.X ||
        RotatedSpatialPosition.Y < MinkowskiCorner.Y ||
        RotatedSpatialPosition.Y > MinkowskiCorner.Y + MinkowskiDimension.Y)
    {
        return false;
    }

    return true;
}

void
UpdateSpatialVelocity(spatial* Spatial)
{
    Spatial->Velocity += Spatial->Acceleration;
    if (Spatial->Acceleration.X > 0.0f &&
        Spatial->Velocity.X > Spatial->SpeedMax.X)
    {
        Spatial->Velocity.X = Spatial->SpeedMax.X;
    }
    if (Spatial->Acceleration.X < 0.0f &&
        Spatial->Velocity.X < Spatial->SpeedMax.X)
    {
        Spatial->Velocity.X = Spatial->SpeedMax.X;
    }

    if (Spatial->Acceleration.Y > 0.0f &&
        Spatial->Velocity.Y > Spatial->SpeedMax.Y)
    {
        Spatial->Velocity.Y = Spatial->SpeedMax.Y;
    }
    if (Spatial->Acceleration.Y < 0.0f &&
        Spatial->Velocity.Y < Spatial->SpeedMax.Y)
    {
        Spatial->Velocity.Y = Spatial->SpeedMax.Y;
    }
}

void
UpdateSpatialPosition(spatial* Spatial)
{
    Spatial->Position += Spatial->Velocity;
}

void
UpdateSpatial(spatial* Spatial)
{
    UpdateSpatialVelocity(Spatial);
    UpdateSpatialPosition(Spatial);
}
