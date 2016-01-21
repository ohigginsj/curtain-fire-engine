#pragma once

#include "game_math.h"

typedef v4 color;

enum color_tint
{
    ColorTint_Mod,
    ColorTint_Luminosity,
};

struct color_spec
{
    color Color;
    color_tint Tint;
};

inline color
Color(real32 R, real32 G, real32 B, real32 A)
{
    color Result;

    Result.R = R;
    Result.G = G;
    Result.B = B;
    Result.A = A;

    Clamp(&Result.R, 0.0f, 1.0f);
    Clamp(&Result.G, 0.0f, 1.0f);
    Clamp(&Result.B, 0.0f, 1.0f);
    Clamp(&Result.A, 0.0f, 1.0f);

    return Result;
}

inline color
Color(color Color, real32 A)
{
    Color.A = A;
    return Color;
}

inline color_spec
ColorSpec(color Color, color_tint Tint)
{
    color_spec Result;

    Result.Color = Color;
    Result.Tint = Tint;

    return Result;
}

global const color_spec ColorSpec_None = ColorSpec(Color(1.0f, 1.0f, 1.0f, 1.0f), ColorTint_Mod);
