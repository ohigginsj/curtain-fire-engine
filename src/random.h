#pragma once

#include "game_math.h"

#define MT_Size 624
#define MT_Period 397
#define MT_MatrixA 0x9908b0df

// MT19937
struct random_mt
{
    int32 MT[MT_Size];
    int32 MT_Index;
};

#define MT_UpperMask(X) (0x80000000 & X)
#define MT_LowerMask(X) (0x7fffffff & X)
#define MT_IsEven(X) (!(X & 1))

void
MT_Seed(random_mt* RandomMT, uint32 Seed)
{
    RandomMT->MT[0] = Seed;
    RandomMT->MT_Index = 0;

    for (uint32 I = 1;
         I < MT_Size;
         ++I)
    {
        RandomMT->MT[I] = 0x6c078965 * (RandomMT->MT[I - 1] ^ RandomMT->MT[I - 1] >> 30) + I;
    }
}

void
MT_Generate(random_mt* RandomMT)
{
    for (uint32 I = 0;
         I < MT_Size;
         ++I)
    {
        uint32 Y = (MT_UpperMask(RandomMT->MT[I])) + (MT_LowerMask(RandomMT->MT[(I + 1) % MT_Size]));
        RandomMT->MT[I] = RandomMT->MT[(I + MT_Period) % MT_Size] ^ (Y >> 1);
        if (MT_IsEven(Y))
        {
            RandomMT->MT[I] = RandomMT->MT[I] ^ MT_MatrixA;
        }
    }
    RandomMT->MT_Index = 0;
}

uint32
MT_Extract(random_mt* RandomMT)
{
    if (RandomMT->MT_Index >= MT_Size)
    {
        MT_Generate(RandomMT);
    }

    uint32 Y = RandomMT->MT[RandomMT->MT_Index];
    Y ^= Y >> 11;
    Y ^= Y <<  7 & 0x9d2c5680;
    Y ^= Y << 15 & 0xefc60000;
    Y ^= Y >> 18;

    ++RandomMT->MT_Index;
    if (RandomMT->MT_Index == MT_Size)
    {
        RandomMT->MT_Index = 0;
    }

    return Y;
}
