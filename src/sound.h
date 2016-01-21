#pragma once

#include "game.h"
#include "world.h"
#include "demo.h"

const int32 Channel_Invalid = -1;

struct sound
{
    Mix_Chunk* MixChunk;
    bool32 PlayedThisFrame;
    int32 CurrentChannel;
};

struct sound_group
{
    bool32 Initialized;

    sound_id FirstSoundId;
    uint32 Size;
};

void
LoadSound(game_state* GameState, sound_id SoundId, const char* Path)
{
    sound* Sound = &GameState->Assets.Sounds[SoundId];
    Sound->MixChunk = Mix_LoadWAV(Path);
    if (!Sound->MixChunk)
    {
        char ErrorMessage[256];
        sprintf(ErrorMessage, "Could not load sound (%s): %s.", Path, Mix_GetError());
        Assert(ErrorMessage, false);
    }
    Sound->CurrentChannel = Channel_Invalid;
    Sound->PlayedThisFrame = false;
}

void
PlaySound(game_state* GameState, sound_id SoundId, real32 Volume)
{
    Volume *= GameState->GlobalVolume;
    Clamp(&Volume, 0.0f, 1.0f);

    if (Volume > 0.0f)
    {
        sound* Sound = &GameState->Assets.Sounds[SoundId];
        if (Sound->PlayedThisFrame)
        {
            return;
        }

        Sound->CurrentChannel = Mix_PlayChannel(-1, Sound->MixChunk, 0);
        if (Sound->CurrentChannel < 0)
        {
            printf("Could not play sound: %s\n", Mix_GetError());
            return;
        }

        Mix_Volume(Sound->CurrentChannel, (uint32)(Volume * MIX_MAX_VOLUME));
        Sound->PlayedThisFrame = true;
    }
}

bool32
IsPlaying(game_state* GameState, sound_id SoundId)
{
    sound* Sound = &GameState->Assets.Sounds[SoundId];

    if (Sound->CurrentChannel == Channel_Invalid)
    {
        return false;
    }

    return Mix_Playing(Sound->CurrentChannel);
}

void
AddSoundGroup(game_state* GameState, sound_group_id GroupId, sound_id FirstSoundId, uint32 Size)
{
    sound_group* SoundGroup = &GameState->Assets.SoundGroups[GroupId];
    if (!SoundGroup->Initialized)
    {
        SoundGroup->Initialized = true;
        SoundGroup->FirstSoundId = FirstSoundId;
        SoundGroup->Size = Size;
    }
}

void
PlayRandomSound(game_state* GameState, sound_group_id GroupId)
{
    sound_group* SoundGroup = &GameState->Assets.SoundGroups[GroupId];
    if (SoundGroup->Initialized)
    {
        int32 GroupOffset = FloorReal32(RandomRange(GameState, (real32)SoundGroup->Size));
        sound_id SoundId = (sound_id)(SoundGroup->FirstSoundId + GroupOffset);
        PlaySound(GameState, SoundId, 1.0f);
    }
}
