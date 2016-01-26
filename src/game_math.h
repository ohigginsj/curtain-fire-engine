#pragma once

#include <SDL2/SDL.h>

#include "game_platform.h"
#include "random.h"
#include "math.h"

struct v2
{
    real32 X;
    real32 Y;
};

struct v3
{
    real32 X;
    real32 Y;
    real32 Z;
};

union v4
{
    struct
    {
        real32 X;
        real32 Y;
        real32 Z;
        real32 W;
    };

    struct
    {
        real32 R;
        real32 G;
        real32 B;
        real32 A;
    };
};

union rect
{
    struct
    {
        v2 Position;
        v2 Size;
    };

    struct
    {
        real32 X;
        real32 Y;
        real32 W;
        real32 H;
    };
};

const real32 PI = 3.14159265358979323846f;

//////////////////
// Real Numbers //
//////////////////

inline int32
RoundReal64(real64 Value)
{
    int32 Result = (int32)roundf((real32)Value);
    return Result;
}

inline int32
FloorReal64(real64 Value)
{
    int32 Result = (int32)floorf((real32)Value);
    return Result;
}

inline int32
CeilReal64(real64 Value)
{
    int32 Result = (int32)ceilf((real32)Value);
    return Result;
}

inline int32
RoundReal32(real32 Value)
{
    int32 Result = (int32)roundf(Value);
    return Result;
}

inline int32
FloorReal32(real32 Value)
{
    int32 Result = (int32)floorf(Value);
    return Result;
}

inline int32
CeilReal32(real32 Value)
{
    int32 Result = (int32)ceilf(Value);
    return Result;
}

inline real32
Max(real32 A, real32 B)
{
    if (A > B)
    {
        return(A);
    }
    else
    {
        return B;
    }
}

inline real32
Min(real32 A, real32 B)
{
    if (A < B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

inline void
ClampHigh(real32* A, real32 Max)
{
    if (*A > Max)
    {
        *A = Max;
    }
}

inline void
ClampLow(real32* A, real32 Min)
{
    if (*A < Min)
    {
        *A = Min;
    }
}

inline void
Clamp(real32* A, real32 Min, real32 Max)
{
    ClampLow(A, Min);
    ClampHigh(A, Max);
}

inline real32
Abs(real32 A)
{
    if (A < 0)
    {
        return -A;
    }
    else
    {
        return A;
    }
}

inline real32
Lerp(real32 Start, real32 End, real32 Time)
{
    Assert("Time must be less than 1.0f", Time <= 1.0f);
    real32 Result = ((1.0f - Time) * Start) + (Time * End);
    return Result;
}

inline real32
SquareRoot(real32 X)
{
    return (real32)sqrt(X);
}

inline real32
Square(real32 X)
{
    return X * X;
}

inline real32
RadiansToDegrees(real32 X)
{
    return X * 180.0f / PI;
}

inline real32
DegreesToRadians(real32 X)
{
    return X * PI / 180.0f;
}

inline real32
Cos(real32 X)
{
    return (real32)cos(DegreesToRadians(X));
}

inline real32
ACos(real32 X)
{
    return (real32)acos(X);
}

inline real32
ATan2(real32 A, real32 B)
{
    return RadiansToDegrees((real32)atan2(A, B));
}

inline real32
Sin(real32 X)
{
    return (real32)sin(DegreesToRadians(X));
}

inline real32
Exponent(real32 X)
{
    return (real32)exp(X);
}

inline real32
Power(real32 A, real32 B)
{
    real32 Result = 1.0f;

    for (uint32 Index = 0;
         Index < B;
         ++Index)
    {
        Result *= A;
    }

    return Result;
}

real32
GetGreatestMultipleLessThan(real32 Base, real32 Max)
{
    real32 Result = 0;

    while (Result + Base <= Max)
    {
        Result += Base;
    }

    return Result;
}

//////////////
// Vector 2 //
//////////////

inline v2
V2(real32 X = 0.0f, real32 Y = 0.0f)
{
    v2 Result;

    Result.X = X;
    Result.Y = Y;

    return(Result);
}

inline v2
Abs(v2 A)
{
    return V2(Abs(A.X), Abs(A.Y));
}

inline v2
operator*(real32 A, v2 B)
{
    v2 Result;

    Result.X = A*B.X;
    Result.Y = A*B.Y;

    return(Result);
}

inline v2
operator*(v2 B, real32 A)
{
    v2 Result = A*B;

    return(Result);
}

inline v2 &
operator*=(v2 &B, real32 A)
{
    B = A * B;

    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;

    Result.X = -A.X;
    Result.Y = -A.Y;

    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;

    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;

    return(Result);
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;

    return(A);
}


inline bool32
operator==(v2 &A, v2 &B)
{
    return (A.X == B.X) && (A.Y == B.Y);
}

inline v2
Lerp(v2 Start, v2 End, real32 Time)
{
    v2 Result;
    Result.X = Lerp(Start.X, End.X, Time);
    Result.Y = Lerp(Start.Y, End.Y, Time);
    return Result;
}

inline real32
Magnitude(v2 &A)
{
    return SquareRoot(A.X * A.X + A.Y * A.Y);
}

inline v2
Normalize(v2& A)
{
    return (1.0f / Magnitude(A)) * A;
}

inline real32
Dot(v2 &A, v2 &B)
{
    return (A.X * B.X) + (A.Y * B.Y);
}

inline v2
Hadamard(v2 &A, v2 &B)
{
    v2 Result;

    Result.X = A.X * B.X;
    Result.Y = A.Y * B.Y;

    return Result;
}

inline real32
VectorAngle(v2 &A)
{
    return ATan2(A.Y, A.X);
}

inline real32
VectorAngle(v2 &A, v2 &B)
{
    real32 DotProduct = Dot(A, B);
    real32 Determinant = (A.X * B.Y) - (A.Y * B.X);
    return ATan2(Determinant, DotProduct);
}

inline v2
Abs(v2 &A)
{
    v2 Result;

    Result.X = Abs(A.X);
    Result.Y = Abs(A.Y);

    return Result;
}

inline v2
Rescale(v2 &A, real32 Length)
{
    v2 Result;

    real32 Mag = Magnitude(A);
    if (Mag == 0.0f)
    {
        Result = V2(0.0f, 0.0f);
    }
    else
    {
        Result = A * (Length / Mag);
    }

    return Result;
}

inline v2
ClampMagnitude(v2 &A, real32 Value)
{
    v2 Result = {};

    if(Magnitude(A) > Value)
    {
        Result = Rescale(A, Value);
    }
    else
    {
        Result = A;
    }

    return Result;
}

inline v2
ClampBounds(v2 &Vector, rect &Rectangle)
{
    v2 Result = Vector;
    Clamp(&Result.X, Rectangle.X, Rectangle.X + Rectangle.W);
    Clamp(&Result.Y, Rectangle.Y, Rectangle.Y + Rectangle.H);
    return Result;
}

inline v2
Rotate(v2 &A, real32 Degrees)
{
    v2 Result;

    Result.X = A.X * Cos(Degrees) - A.Y * Sin(Degrees);
    Result.Y = A.X * Sin(Degrees) + A.Y * Cos(Degrees);

    return Result;
}

inline v2
RotateAround(v2 &A, v2& Center, real32 Angle)
{
    v2 FromOrigin = A - Center;
    v2 Rotated = Rotate(FromOrigin, Angle);
    v2 Result = Rotated + Center;
    return Result;
}

inline v2
GetDirectionFromAngle(real32 Angle)
{
    v2 UnitVector = V2(1.0f, 0.0f);
    return Rotate(UnitVector, Angle);
}

inline v2
PolarToCartesian(real32 Angle, real32 Length)
{
    v2 Direction = GetDirectionFromAngle(Angle);
    return Rescale(Direction, Length);
}

real32
DistanceSquared(v2 A, v2 B)
{
    v2 Distance = Abs(A - B);
    return Distance.X * Distance.X + Distance.Y * Distance.Y;
}

real32
Distance(v2 A, v2 B)
{
    real32 Distance = DistanceSquared(A, B);
    return (real32)sqrt(Distance);
}

//////////////
// Vector 3 //
//////////////

inline v3
V3(real32 X = 0, real32 Y = 0, real32 Z = 0)
{
    v3 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;

    return(Result);
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;

    return(A);
}

////////////////
// Rectangles //
////////////////

inline rect
Rect(real32 X, real32 Y, real32 W, real32 H)
{
    rect Result;

    Result.X = X;
    Result.Y = Y;
    Result.W = W;
    Result.H = H;

    return(Result);
}

inline rect
Rect(v2 Position, v2 Size)
{
    rect Result;

    Result.Position = Position;
    Result.Size = Size;

    return(Result);
}

inline rect
operator+(rect& Rect, v2& Vector)
{
    rect Result;

    Result.Position = Rect.Position + Vector;
    Result.Size = Rect.Size;

    return Result;
}

inline rect
GetRectFromCenter(v2& Center, v2& HalfDimension)
{
    rect Result = Rect(Center.X - HalfDimension.X,
                       Center.Y - HalfDimension.Y,
                       2.0f * HalfDimension.X,
                       2.0f * HalfDimension.Y);
    return Result;
}
