#pragma once

#include "api.h"
#include "world.h"

struct texture
{
    GLuint GL_Id;
    GLuint Width;
    GLuint Height;
    GLuint InternalFormat;
    GLuint ImageFormat;
    GLuint WrapS;
    GLuint WrapT;
    GLuint FilterMin;
    GLuint FilterMag;
};

struct texture_frame
{
    real32 AtlasOffsetX;
    real32 AtlasOffsetY;
};

struct texture_atlas
{
    GLuint TextureId;
    uint32 Width;
    uint32 Height;
    GLuint InternalFormat;
    GLuint ImageFormat;
    GLuint WrapS;
    GLuint WrapT;
    GLuint FilterMin;
    GLuint FilterMag;

    uint32 FrameWidth;
    uint32 FrameHeight;
    uint32 FrameCount;
    texture_frame Frames[128];
};

void
InitializeTextureAtlas(texture_atlas* TextureAtlas)
{
    glGenTextures(1, &TextureAtlas->TextureId);
    TextureAtlas->Width = 0;
    TextureAtlas->Height = 0;
    TextureAtlas->FrameWidth = 0;
    TextureAtlas->FrameHeight = 0;
    TextureAtlas->InternalFormat = GL_RGBA;
    TextureAtlas->ImageFormat = GL_RGBA;
    TextureAtlas->WrapS = GL_CLAMP_TO_BORDER;
    TextureAtlas->WrapT = GL_CLAMP_TO_BORDER;
    TextureAtlas->FilterMin = GL_NEAREST;
    TextureAtlas->FilterMag = GL_NEAREST;
}

void
GenerateTextureAtlas(texture_atlas* TextureAtlas,
                     GLuint Width,
                     GLuint Height,
                     GLuint FrameWidth,
                     GLuint FrameHeight,
                     unsigned char* Data)
{
    TextureAtlas->Width = Width;
    TextureAtlas->Height = Height;

    TextureAtlas->FrameWidth = FrameWidth;
    TextureAtlas->FrameHeight = FrameHeight;

    uint32 FramesX = (uint32)(Width / FrameWidth);
    uint32 FramesY = (uint32)(Height / FrameHeight);
    TextureAtlas->FrameCount = FramesX * FramesY;

    glBindTexture(GL_TEXTURE_2D, TextureAtlas->TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, TextureAtlas->InternalFormat, Width, Height, 0, TextureAtlas->ImageFormat, GL_UNSIGNED_BYTE, Data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureAtlas->WrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureAtlas->WrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureAtlas->FilterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureAtlas->FilterMag);

    glBindTexture(GL_TEXTURE_2D, TextureAtlas->TextureId);
}

void
BindTextureAtlas(texture_atlas* TextureAtlas)
{
    glBindTexture(GL_TEXTURE_2D, TextureAtlas->TextureId);
}

void
InitializeTexture(texture* Texture,
                  texture_filter TextureFilter)
{
    glGenTextures(1, &Texture->GL_Id);
    Texture->Width = 0;
    Texture->Height = 0;
    Texture->InternalFormat = GL_RGBA;
    Texture->ImageFormat = GL_RGBA;
    Texture->WrapS = GL_CLAMP_TO_BORDER;
    Texture->WrapT = GL_CLAMP_TO_BORDER;

    switch (TextureFilter)
    {
        case TextureFilter_Nearest:
        {
            Texture->FilterMin = GL_NEAREST;
            Texture->FilterMag = GL_NEAREST;
        } break;
        case TextureFilter_Linear:
        {
            Texture->FilterMin = GL_LINEAR;
            Texture->FilterMag = GL_LINEAR;
        } break;
    }
}

void
GenerateTexture(texture* Texture,
                GLuint Width,
                GLuint Height,
                unsigned char* Data)
{
    Texture->Width = Width;
    Texture->Height = Height;

    glBindTexture(GL_TEXTURE_2D, Texture->GL_Id);
    glTexImage2D(GL_TEXTURE_2D, 0, Texture->InternalFormat, Width, Height, 0, Texture->ImageFormat, GL_UNSIGNED_BYTE, Data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Texture->WrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Texture->WrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture->FilterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Texture->FilterMag);

    glBindTexture(GL_TEXTURE_2D, Texture->GL_Id);
}

void
BindTexture(texture* Texture)
{
    glBindTexture(GL_TEXTURE_2D, Texture->GL_Id);
}

// API
void
LoadTexture(game_state* GameState,
            texture_id TextureId,
            const char* File,
            texture_filter TextureFilter)
{
    texture* Texture = &GameState->Assets.Textures[TextureId];
    InitializeTexture(Texture, TextureFilter);
    int Width;
    int Height;
    int ComponentsPerPixel;
    unsigned char* ImageData = stbi_load(File, &Width, &Height, &ComponentsPerPixel, 4);
    Assert("Could not load image.", ImageData);
    GenerateTexture(Texture, Width, Height, ImageData);
    stbi_image_free(ImageData);
}

void
LoadTextureAtlas(const char* File, texture_atlas* TextureAtlas, uint32 FrameWidth, uint32 FrameHeight)
{
    InitializeTextureAtlas(TextureAtlas);
    int Width;
    int Height;
    int ComponentsPerPixel;
    unsigned char* ImageData = stbi_load(File, &Width, &Height, &ComponentsPerPixel, 4);
    Assert("Could not load image.", ImageData);
    GenerateTextureAtlas(TextureAtlas, Width, Height, FrameWidth, FrameHeight, ImageData);
    stbi_image_free(ImageData);
}

void
LoadScrollingBackground(game_state* GameState,
                        texture_id TextureId,
                        int32 RepeatY,
                        real32 ScrollSpeed)
{
    scrolling_background* Background = GameState->World->ScrollingBackground;
    Background->TextureId = TextureId;
    Background->TextureRepeatY = RepeatY;
    Background->ScrollSpeed = ScrollSpeed;
    texture* Texture = &GameState->Assets.Textures[TextureId];
    Background->SourceY = Texture->Height - WorldSizeY;
}

v2
GetTextureCenter(game_state* GameState, texture_id TextureId)
{
    texture* Texture = &GameState->Assets.Textures[TextureId];
    return V2((real32)Texture->Width / 2.0f, (real32)Texture->Height / 2.0f);
}
