#pragma once

#include "shader.h"
#include "texture.h"

const uint32 ScreenShakeCountMax = 16;
const uint32 RenderEntryCountMax = 16384;
const uint32 RenderTextLengthMax = 64;

struct text_character
{
    v2 Size;
    v2 Bearing;
    GLfloat Advance;
    GLfloat AtlasOffsetX;
};

struct font_atlas
{
    GLuint TextureId;
    uint32 Width;
    uint32 Height;
    text_character Characters[128];
};

struct text_renderer
{
    shader* Shader;
    GLuint VAO;
    GLuint VBO;
    font_atlas FontAtlas;
};

struct texture_renderer
{
    shader* Shader;
    GLuint QuadVAO;
    GLuint VBO;
};

struct rectangle_renderer
{
    shader* Shader;
    GLuint VAO;
    GLuint VBO;
};

struct circle_renderer
{
    shader* Shader;
    GLuint VAO;
    GLuint VBO;
};

struct polygon4_renderer
{
    shader* Shader;
    GLuint VAO;
    GLuint VBO;
};

struct screen_shake
{
    bool32 Active;
    real32 Magnitude;
    real32 RemainingTime;
};

struct post_processor
{
    screen_shake ScreenShakes[ScreenShakeCountMax];
    shader* Shader;
    texture Texture;
    GLuint Width;
    GLuint Height;
    GLfloat ShakeMagnitude;
    GLuint MS_FBO;
    GLuint FBO;
    GLuint RBO;
    GLuint VAO;
};

enum render_entry_type
{
    RenderEntry_Texture,
    RenderEntry_Text,
    RenderEntry_Rectangle,
    RenderEntry_Circle,
    RenderEntry_Polygon4,
};

struct render_entry_texture
{
    texture_id TextureId;
    real32 Angle;
    v2 Center;
    color_spec ColorSpec;
    rect SourceRect;
    rect DestRect;
    blend_mode BlendMode;
};

struct render_entry_text
{
    char Text[RenderTextLengthMax];
    v2 Position;
    color Color;
    bool32 Centered;
};

struct render_entry_rectangle
{
    bool32 Filled;
    color Color;
    rect Rect;
};

struct render_entry_circle
{
    bool32 Filled;
    color Color;
    v2 Position;
    real32 Radius;
};

struct render_entry_polygon4
{
    bool32 Filled;
    color Color;
    v2 A;
    v2 B;
    v2 C;
    v2 D;
};

struct render_entry
{
    int32 Layer;

    render_entry_type Type;
    union
    {
        render_entry_texture Texture;
        render_entry_text Text;
        render_entry_rectangle Rectangle;
        render_entry_circle Circle;
        render_entry_polygon4 Polygon4;
    };
};

struct renderer
{
    texture_renderer TextureRenderer;
    text_renderer TextRenderer;
    rectangle_renderer RectangleRenderer;
    circle_renderer CircleRenderer;
    polygon4_renderer Polygon4Renderer;
    post_processor PostProcessor;

    uint32 EntryCount;
    render_entry Entries[RenderEntryCountMax];
};

struct animation_spec
{
    texture_id TextureId;
    uint32 FrameCount;
    int32 FrameTime;
    rect Frames[64];
};

struct animation
{
    animation_spec_id SpecId;
    real32 TimeSinceLastFrame;
    int32 FrameIndex;
    bool32 Complete;
};

struct sprite
{
    v2 RenderOffset;

    sprite_type Type;
    union
    {
        texture_id TextureId;
        animation Animation;
    };
};

void
LoadFont(transient_state* TransientState,
         text_renderer* TextRenderer,
         const char* Path, uint32 Size)
{
    FT_Library FreeType;
    FT_Init_FreeType(&FreeType);
    FT_Face Face;
    FT_New_Face(FreeType, Path, 0, &Face);
    FT_Set_Pixel_Sizes(Face, 0, Size);

    uint32 MaxHeight = 0;
    real32 TotalWidth = 0.0f;

    for (GLubyte Character = 0;
         Character < 128;
         ++Character)
    {
        FT_Load_Char(Face, Character, FT_LOAD_RENDER);
        real32 CharacterWidth = Face->glyph->bitmap.width;
        real32 CharacterRows = Face->glyph->bitmap.rows;
        real32 CharacterLeft = Face->glyph->bitmap_left;
        real32 CharacterTop = Face->glyph->bitmap_top;
        real32 CharacterAdvance = Face->glyph->advance.x >> 6;

        text_character TextCharacter = { V2(CharacterWidth, CharacterRows),
                                         V2(CharacterLeft, CharacterTop),
                                         CharacterAdvance,
                                         TotalWidth };
        TextRenderer->FontAtlas.Characters[Character] = TextCharacter;

        if (CharacterRows > MaxHeight)
        {
            MaxHeight = CharacterRows;
        }

        TotalWidth += CharacterWidth + 1;
    }

    TextRenderer->FontAtlas.Width = TotalWidth;
    TextRenderer->FontAtlas.Height = MaxHeight;

    Assert("Couldn't get max height.\n", MaxHeight != 0);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &TextRenderer->FontAtlas.TextureId);
    glBindTexture(GL_TEXTURE_2D, TextRenderer->FontAtlas.TextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO: Better way to clear texture to black??
    temporary_memory ClearMemory = BeginTemporaryMemory(&TransientState->Memory);
        uint32 Pixels = TotalWidth * MaxHeight * 4;
        GLuint* Black = PushArray(ClearMemory.Memory, Pixels, GLuint);
        for (uint32 Pixel = 0;
             Pixel < Pixels;
             ++Pixel)
        {
            Black[Pixel] = 0;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     TotalWidth, MaxHeight,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     Black);
    EndTemporaryMemory(&ClearMemory);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    uint32 TextureAtlasFillX = 0;
    for (GLubyte Character = 0;
         Character < 128;
         ++Character)
    {
        FT_Load_Char(Face, Character, FT_LOAD_RENDER);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        TextureAtlasFillX, 0,
                        Face->glyph->bitmap.width,
                        Face->glyph->bitmap.rows,
                        GL_RED, GL_UNSIGNED_BYTE, Face->glyph->bitmap.buffer);
        TextureAtlasFillX += Face->glyph->bitmap.width + 1;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(Face);
    FT_Done_FreeType(FreeType);
}

void
InitializeTextRenderer(transient_state* TransientState,
                       text_renderer* TextRenderer,
                       shader* Shader,
                       glm::mat4 Projection,
                       const char* FontPath,
                       int32 FontSize)
{
    TextRenderer->Shader = Shader;
    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    // Create buffers
    glGenVertexArrays(1, &TextRenderer->VAO);
    glGenBuffers(1, &TextRenderer->VBO);

    glBindVertexArray(TextRenderer->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, TextRenderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, 0, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    // Cleanup
    glBindVertexArray(0);

    LoadFont(TransientState, TextRenderer, FontPath, FontSize);
}

void
InitializeTextureRenderer(texture_renderer* Renderer,
                          shader* Shader,
                          glm::mat4 Projection)
{
    Renderer->Shader = Shader;
    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    // Create Buffers
    glGenVertexArrays(1, &Renderer->QuadVAO);
    glGenBuffers(1, &Renderer->VBO);

    // Even though these will be set later, we allocate the correct size now
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 6, 0, GL_DYNAMIC_DRAW);

    // Bind VAO
    glBindVertexArray(Renderer->QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    // Unbind VAO
    glBindVertexArray(0);
}

void
InitializeRectangleRenderer(rectangle_renderer* Renderer,
                           shader* Shader,
                           glm::mat4 Projection)
{
    Renderer->Shader = Shader;
    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    // Create Buffers
    glGenVertexArrays(1, &Renderer->VAO);
    glGenBuffers(1, &Renderer->VBO);

    // Bind VAO
    glBindVertexArray(Renderer->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, 0, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);
}

void
InitializeCircleRenderer(circle_renderer* Renderer,
                         shader* Shader,
                         glm::mat4 Projection)
{
    Renderer->Shader = Shader;
    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    // Create Buffers
    glGenVertexArrays(1, &Renderer->VAO);
    glGenBuffers(1, &Renderer->VBO);

    // Bind VAO
    glBindVertexArray(Renderer->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 7, 0, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);
}

void
InitializePolygon4Renderer(polygon4_renderer* Renderer,
                           shader* Shader,
                           glm::mat4 Projection)
{
    Renderer->Shader = Shader;
    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    // Create Buffers
    glGenVertexArrays(1, &Renderer->VAO);
    glGenBuffers(1, &Renderer->VBO);

    // Bind VAO
    glBindVertexArray(Renderer->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, 0, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
        glEnableVertexAttribArray(4);

    // Unbind VAO
    glBindVertexArray(0);
}

void
ResizePostProcessor(post_processor* PostProcessor,
                    uint32 Width,
                    uint32 Height)
{
    PostProcessor->Width = Width;
    PostProcessor->Height = Height;

    glBindFramebuffer(GL_FRAMEBUFFER, PostProcessor->MS_FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, PostProcessor->RBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGB, PostProcessor->Width, PostProcessor->Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, PostProcessor->RBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Could not initialize MS_FBO.\n");
    }

    InitializeTexture(&PostProcessor->Texture, TextureFilter_Nearest);
    GenerateTexture(&PostProcessor->Texture, Width, Height, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, PostProcessor->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PostProcessor->Texture.GL_Id, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Could not initialize FBO.\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
InitializePostProcessor(post_processor* PostProcessor,
                        shader* Shader,
                        uint32 Width,
                        uint32 Height,
                        glm::mat4 Projection)
{
    PostProcessor->Shader = Shader;
    PostProcessor->ShakeMagnitude = 0.0f;

    ShaderUse(Shader);
    ShaderSetMatrix4f(Shader, "Projection", &Projection);

    GLfloat Offset = 1.0f / 300.0f;
    GLfloat Offsets[9][2] = {
        { -Offset,  Offset  },  // top-left
        {  0.0f,    Offset  },  // top-center
        {  Offset,  Offset  },  // top-right
        { -Offset,  0.0f    },  // center-left
        {  0.0f,    0.0f    },  // center-center
        {  Offset,  0.0f    },  // center - right
        { -Offset, -Offset  },  // bottom-left
        {  0.0f,   -Offset  },  // bottom-center
        {  Offset, -Offset  }   // bottom-right
    };
    glUniform2fv(glGetUniformLocation(Shader->GL_Id, "Offsets"), 9, (GLfloat*)Offsets);
    GLfloat BlurKernel[9] = {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
        2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
    };
    glUniform1fv(glGetUniformLocation(Shader->GL_Id, "BlurKernel"), 9, BlurKernel);

    glGenFramebuffers(1, &PostProcessor->MS_FBO);
    glGenFramebuffers(1, &PostProcessor->FBO);
    glGenRenderbuffers(1, &PostProcessor->RBO);

    ResizePostProcessor(PostProcessor, Width, Height);

#if 1
    v2 QuadSize = V2((real32)LogicalWindowSizeX,
                     (real32)LogicalWindowSizeY);
#else
    v2 QuadSize = V2((real32)LogicalWindowSizeX + 2*RenderPadding,
                     (real32)LogicalWindowSizeY + 2*RenderPadding);
#endif

    // Rendering data
    GLuint VBO;
    GLfloat Vertices[] = {
        // Pos        // Tex
#if 1
        0.0f,       QuadSize.Y, 0.0f, 0.0f,
        QuadSize.X, 0.0f,       1.0f, 1.0f,
        0.0f,       0.0f,       0.0f, 1.0f,

        0.0f,       QuadSize.Y, 0.0f, 0.0f,
        QuadSize.X, QuadSize.Y, 1.0f, 0.0f,
        QuadSize.X, 0.0f,       1.0f, 1.0f
#else
        -RenderPadding, QuadSize.Y, 0.0f, 0.0f,
        QuadSize.X,     -RenderPadding,       1.0f, 1.0f,
        -RenderPadding, -RenderPadding,       0.0f, 1.0f,

        -RenderPadding, QuadSize.Y, 0.0f, 0.0f,
        QuadSize.X,     QuadSize.Y, 1.0f, 0.0f,
        QuadSize.X,     -RenderPadding,       1.0f, 1.0f
#endif
    };

    glGenVertexArrays(1, &PostProcessor->VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glBindVertexArray(PostProcessor->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (GLvoid*)0);

    // Cleanup
    glBindVertexArray(0);
}

void
BeginPostProcessor(post_processor* PostProcessor)
{
    glBindFramebuffer(GL_FRAMEBUFFER, PostProcessor->MS_FBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void
EndPostProcessor(post_processor* PostProcessor)
{
    glBlitNamedFramebuffer(PostProcessor->MS_FBO,
                           PostProcessor->FBO,
                           0, 0, PostProcessor->Width, PostProcessor->Height,
                           0, 0, PostProcessor->Width, PostProcessor->Height,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
RenderPostProcessor(post_processor* PostProcessor,
                    GLfloat Time,
                    GLfloat ShakeMagnitude)
{
    shader* Shader = PostProcessor->Shader;
    ShaderUse(Shader);
    ShaderSetFloat(Shader, "Time", Time);
    ShaderSetFloat(Shader, "ShakeMagnitude", ShakeMagnitude);

    glActiveTexture(GL_TEXTURE0);
    BindTexture(&PostProcessor->Texture);
    glBindVertexArray(PostProcessor->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void
UpdateAnimation(game_state* GameState,
                animation* Animation)
{
    animation_spec* Spec = &GameState->Assets.Animations[Animation->SpecId];
    ++Animation->TimeSinceLastFrame;
    if(Animation->TimeSinceLastFrame >= Spec->FrameTime)
    {
        Animation->TimeSinceLastFrame = 0;
        ++Animation->FrameIndex;
    }
    if (Animation->FrameIndex >= (int32)Spec->FrameCount)
    {
        Animation->FrameIndex = 0;
        Animation->Complete = true;
    }
}

void
InitializeSprite(sprite* Sprite,
                 texture_id TextureId)
{
    Sprite->Type = SpriteType_Static;
    Sprite->TextureId = TextureId;
}

void
InitializeSprite_Animated(sprite* Sprite,
                          animation_spec_id AnimationSpecId)
{
    Sprite->Type = SpriteType_Animation;
    Sprite->Animation.SpecId = AnimationSpecId;
    Sprite->Animation.FrameIndex = -1;
    Sprite->Animation.Complete = false;
}

void
UpdateSprite(game_state* GameState, sprite* Sprite)
{
    if (Sprite->Type == SpriteType_Animation)
    {
        if (!Sprite->Animation.Complete)
        {
            UpdateAnimation(GameState, &Sprite->Animation);
        }
    }
}

void
ScreenShake(renderer* Renderer, real32 Magnitude, real32 Duration)
{
    Assert("Duration must be positive", Duration > 0.0f);
    if (Duration < 0.0f)
    {
        return;
    }

    bool32 Found = false;

    for (uint32 ScreenShakeIndex = 0;
         ScreenShakeIndex < ScreenShakeCountMax;
         ++ScreenShakeIndex)
    {
        screen_shake* ScreenShake = Renderer->PostProcessor.ScreenShakes + ScreenShakeIndex;
        if (!ScreenShake->Active)
        {
            ScreenShake->Active = true;
            ScreenShake->Magnitude = Magnitude;
            ScreenShake->RemainingTime = Duration;

            Found = true;
            break;
        }
    }

    Assert("Out of screen shake memory.", Found);
}

void
ClearScreenShake(game_state* GameState)
{
    for (uint32 ScreenShakeIndex = 0;
         ScreenShakeIndex < ScreenShakeCountMax;
         ++ScreenShakeIndex)
    {
        screen_shake* ScreenShake = GameState->Renderer->PostProcessor.ScreenShakes + ScreenShakeIndex;
        ScreenShake->Active = false;
    }
}

void
UpdateScreenShakes(post_processor* PostProcessor,
                   real32 DeltaTime)
{
    for (uint32 ScreenShakeIndex = 0;
         ScreenShakeIndex < ScreenShakeCountMax;
         ++ScreenShakeIndex)
    {
        screen_shake* ScreenShake = PostProcessor->ScreenShakes + ScreenShakeIndex;
        if (ScreenShake->Active)
        {
            ScreenShake->RemainingTime -= DeltaTime;
            if (ScreenShake->RemainingTime <= 0.0f)
            {
                ScreenShake->Active = false;
            }
        }
    }
}

real32
GetLargestScreenShakeMagnitude(post_processor* PostProcessor)
{
    real32 LargestMagnitude = 0.0f;

    for (uint32 ScreenShakeIndex = 0;
         ScreenShakeIndex < ScreenShakeCountMax;
         ++ScreenShakeIndex)
    {
        screen_shake* ScreenShake = PostProcessor->ScreenShakes + ScreenShakeIndex;
        if (ScreenShake->Active)
        {
            if (ScreenShake->Magnitude > LargestMagnitude)
            {
                LargestMagnitude = ScreenShake->Magnitude;
            }
        }
    }

    return LargestMagnitude;
}

rect
GetScaledCenteredRect(v2 Position, v2 Size, real32 Scale)
{
    real32 SizeX = Size.X * Scale;
    real32 SizeY = Size.Y * Scale;
    rect Result = Rect(Position.X - (SizeX / 2.0f),
                       Position.Y - (SizeY / 2.0f),
                       SizeX,
                       SizeY);
    return Result;
}

rect
GetFullTextureRect(game_state* GameState,
                   texture_id TextureId)
{
    texture* Texture = &GameState->Assets.Textures[TextureId];
    rect Result = Rect(0, 0, Texture->Width, Texture->Height);
    return Result;
}

v2
GetTextureCenter(game_state* GameState, texture_id TextureId)
{
    texture* Texture = &GameState->Assets.Textures[TextureId];
    return V2((real32)Texture->Width / 2.0f, (real32)Texture->Height / 2.0f);
}

// Centered
void
PushTextureEx(renderer* Renderer,
              texture_id TextureId,
              rect SourceRect,
              rect DestRect,
              v2 Center,
              real32 Angle,
              color_spec ColorSpec,
              int32 Layer,
              blend_mode BlendMode)
{
    render_entry* NewEntry = &Renderer->Entries[Renderer->EntryCount];
    NewEntry->Layer = Layer;

    render_entry_texture* TextureEntry = &NewEntry->Texture;
    TextureEntry->TextureId = TextureId;
    TextureEntry->Center = Center;
    TextureEntry->Angle = Angle;
    TextureEntry->ColorSpec = ColorSpec;
    TextureEntry->SourceRect = SourceRect;
    TextureEntry->DestRect = DestRect;
    TextureEntry->BlendMode = BlendMode;
    NewEntry->Type = RenderEntry_Texture;
    ++Renderer->EntryCount;
    Assert("Out of render entry memory.", Renderer->EntryCount < RenderEntryCountMax);
}

void
PushTextureEx(renderer* Renderer,
              texture_id TextureId,
              rect SourceRect,
              v2 Position,
              real32 Scale,
              v2 Center,
              real32 Angle,
              color_spec ColorSpec,
              int32 Layer,
              blend_mode BlendMode)
{
    rect DestRect = GetScaledCenteredRect(Position,
                                          V2(SourceRect.W, SourceRect.H),
                                          Scale);
    PushTextureEx(Renderer, TextureId, SourceRect, DestRect,
                  Center, Angle, ColorSpec, Layer, BlendMode);
}

void
PushTexture(renderer* Renderer,
            texture_id TextureId,
            rect SourceRect,
            v2 Position,
            real32 Scale,
            int32 Layer)
{
    PushTextureEx(Renderer,
                  TextureId,
                  SourceRect,
                  Position,
                  Scale,
                  V2(0.0f, 0.0f),
                  0.0f,
                  ColorSpec_None,
                  Layer,
                  BlendMode_Blend);
}

void
PushAnimation(game_state* GameState,
              animation* Animation,
              v2 Position,
              real32 Scale,
              v2 Center,
              real32 Angle,
              color_spec ColorSpec,
              int32 Layer,
              blend_mode BlendMode)
{
    animation_spec* Spec = GameState->Assets.Animations + Animation->SpecId;
    rect SourceRect = Spec->Frames[Animation->FrameIndex];
    PushTextureEx(GameState->Renderer,
                  Spec->TextureId,
                  SourceRect,
                  Position,
                  Scale,
                  V2(0.0f, 0.0f),
                  0.0f,
                  ColorSpec,
                  Layer,
                  BlendMode);
}

void
PushTexture(renderer* Renderer,
            texture_id TextureId,
            rect SourceRect,
            rect DestRect,
            int32 Layer)
{
    PushTextureEx(Renderer,
                  TextureId,
                  SourceRect,
                  DestRect,
                  V2(0.0f, 0.0f),
                  0.0f,
                  ColorSpec_None,
                  Layer,
                  BlendMode_Blend);
}

void
PushSprite(game_state* GameState,
           sprite* Sprite,
           v2 Position,
           real32 Scale,
           v2 Center,
           real32 Angle,
           color_spec ColorSpec,
           int32 Layer,
           blend_mode BlendMode)
{
    if (Sprite->Type == SpriteType_Static)
    {
        PushTextureEx(GameState->Renderer,
                      Sprite->TextureId,
                      GetFullTextureRect(GameState, Sprite->TextureId),
                      Position,
                      Scale,
                      V2(0.0f, 0.0f),
                      0.0f,
                      ColorSpec,
                      Layer,
                      BlendMode);
    }
    else if (Sprite->Type == SpriteType_Animation)
    {
        if (!Sprite->Animation.Complete)
        {
            PushAnimation(GameState,
                          &Sprite->Animation,
                          Position,
                          Scale,
                          V2(0.0f, 0.0f),
                          0.0f,
                          ColorSpec,
                          Layer,
                          BlendMode);
        }
    }
}

void
PushRect(renderer* Renderer,
         rect Rect,
         color Color,
         bool32 Filled,
         int32 Layer)
{
    render_entry* NewEntry = &Renderer->Entries[Renderer->EntryCount];
    NewEntry->Layer = Layer;

    render_entry_rectangle* RectangleEntry = &NewEntry->Rectangle;
    RectangleEntry->Filled = Filled;
    RectangleEntry->Color = Color;
    RectangleEntry->Rect = Rect;

    NewEntry->Type = RenderEntry_Rectangle;
    ++Renderer->EntryCount;
    Assert("Out of render entry memory.", Renderer->EntryCount < RenderEntryCountMax);
}

void
PushText(renderer* Renderer,
         const char* Text,
         v2 Position,
         color Color,
         bool32 Centered,
         int32 Layer)
{
    Assert("String is too long to render.", strlen(Text) < RenderTextLengthMax);

    text_renderer* TextRenderer = &Renderer->TextRenderer;
    render_entry* NewEntry = &Renderer->Entries[Renderer->EntryCount];
    NewEntry->Layer = Layer;

    real32 Scale = 1.0f;

    render_entry_text* TextEntry = &NewEntry->Text;
    strcpy(TextEntry->Text, Text);
    TextEntry->Color = Color;
    TextEntry->Centered = Centered;

    // Determine the size of the rendered text
    v2 TextRenderSize = V2(0.0f, 0.0f);
    for (uint32 CharacterIndex = 0;
         CharacterIndex < strlen(TextEntry->Text);
         ++CharacterIndex)
    {
        char Character = TextEntry->Text[CharacterIndex];
        text_character TextCharacter = TextRenderer->FontAtlas.Characters[(int32)Character];
        TextRenderSize.X += TextCharacter.Advance * Scale;
        TextRenderSize.Y = Max(TextRenderSize.Y, TextCharacter.Size.Y * Scale);
    }

    // So that we can center it
    if (TextEntry->Centered)
    {
        TextEntry->Position = Position - V2(TextRenderSize.X * 0.5f, 0.0f);
    }
    else
    {
        TextEntry->Position = Position;
    }

    NewEntry->Type = RenderEntry_Text;
    ++Renderer->EntryCount;
    Assert("Out of render entry memory.", Renderer->EntryCount < RenderEntryCountMax);
}

void
RenderText(game_state* GameState,
           const char* Text,
           v2 Position,
           color Color,
           bool32 Centered,
           int32 Layer)
{
    PushText(GameState->Renderer,
             Text,
             Position,
             Color,
             Centered,
             Layer);
}

void
PushPolygon4(renderer* Renderer,
             v2 A, v2 B, v2 C, v2 D,
             color Color,
             bool32 Filled,
             int32 Layer)
{
    render_entry* NewEntry = &Renderer->Entries[Renderer->EntryCount];
    NewEntry->Layer = Layer;

    render_entry_polygon4* Polygon4Entry = &NewEntry->Polygon4;
    Polygon4Entry->Filled = Filled;
    Polygon4Entry->Color = Color;
    Polygon4Entry->A = A;
    Polygon4Entry->B = B;
    Polygon4Entry->C = C;
    Polygon4Entry->D = D;

    NewEntry->Type = RenderEntry_Polygon4;
    ++Renderer->EntryCount;
    Assert("Out of render entry memory.", Renderer->EntryCount < RenderEntryCountMax);
}

void
PushCircle(renderer* Renderer,
           v2 Position,
           real32 Radius,
           color Color,
           bool32 Filled,
           int32 Layer)
{
    render_entry* NewEntry = &Renderer->Entries[Renderer->EntryCount];
    NewEntry->Layer = Layer;

    render_entry_circle* CircleEntry = &NewEntry->Circle;
    CircleEntry->Filled = Filled;
    CircleEntry->Color = Color;
    CircleEntry->Position = Position;
    CircleEntry->Radius = Radius;

    NewEntry->Type = RenderEntry_Circle;
    ++Renderer->EntryCount;
    Assert("Out of render entry memory.", Renderer->EntryCount < RenderEntryCountMax);
}

// TODO: Make this take a world and a renderer
void
Render(game_state* GameState)
{
    game_mode_world* World = GameState->World;

    renderer* Renderer = GameState->Renderer;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Sort the entries by Layer (insertion sort)
    render_entry* Entries = Renderer->Entries;
    for (uint32 Index = 1;
         Index < Renderer->EntryCount;
         ++Index)
    {
        int32 SwapIndex = Index;
        while (SwapIndex > 0 && Entries[SwapIndex-1].Layer < Entries[SwapIndex].Layer)
        {
            render_entry Temp = Entries[SwapIndex];
            Entries[SwapIndex] = Entries[SwapIndex-1];
            Entries[SwapIndex-1] = Temp;
            SwapIndex -= 1;
        }
    }

    BeginPostProcessor(&Renderer->PostProcessor);

    // Render the ordered entries
    for (uint32 RenderEntryIndex = 0;
         RenderEntryIndex < Renderer->EntryCount;
         ++RenderEntryIndex)
    {
        render_entry* Entry = &Renderer->Entries[RenderEntryIndex];
        switch (Entry->Type)
        {
            case RenderEntry_Texture:
            {
                render_entry_texture* TextureEntry = &Entry->Texture;
                texture* Texture = &GameState->Assets.Textures[TextureEntry->TextureId];
                texture_renderer* TextureRenderer = &Renderer->TextureRenderer;
                rect SourceRect = TextureEntry->SourceRect;

                struct frame_point
                {
                    GLfloat X;
                    GLfloat Y;
                    GLfloat S;
                    GLfloat T;
                } FrameCoordinates[6];

                real32 TextureCoordinateLeftX = SourceRect.X / (real32)Texture->Width;
                real32 TextureCoordinateRightX = (SourceRect.X + SourceRect.W) / (real32)Texture->Width;
                // TODO: Do this in the shader...
                // NOTE: Bottom and top are reversed because Y points down in our renderer
                real32 TextureCoordinateTopY = (SourceRect.Y + SourceRect.H) / (real32)Texture->Height;
                real32 TextureCoordinateBottomY = SourceRect.Y / (real32)Texture->Height;
#if 0
                if (TextureEntry->FlipX)
                {
                    real32 Temp = TextureCoordinateLeftX;
                    TextureCoordinateLeftX = TextureCoordinateRightX;
                    TextureCoordinateRightX = Temp;
                }
#endif
                FrameCoordinates[0] = {
                    0.0f,
                    1.0f,
                    TextureCoordinateLeftX,
                    TextureCoordinateTopY
                };

                FrameCoordinates[1] = {
                    0.0f,
                    0.0f,
                    TextureCoordinateLeftX,
                    TextureCoordinateBottomY
                };

                FrameCoordinates[2] = {
                    1.0f,
                    0.0f,
                    TextureCoordinateRightX,
                    TextureCoordinateBottomY
                };

                FrameCoordinates[3] = {
                    0.0f,
                    1.0f,
                    TextureCoordinateLeftX,
                    TextureCoordinateTopY
                };

                FrameCoordinates[4] = {
                    1.0f,
                    0.0f,
                    TextureCoordinateRightX,
                    TextureCoordinateBottomY
                };

                FrameCoordinates[5] = {
                    1.0f,
                    1.0f,
                    TextureCoordinateRightX,
                    TextureCoordinateTopY
                };

                if (TextureEntry->BlendMode == BlendMode_Add)
                {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                }

                shader* Shader = TextureRenderer->Shader;
                ShaderUse(Shader);

                // Translation
                glm::mat4 Model;
                Model = glm::translate(Model, glm::vec3(TextureEntry->DestRect.X, TextureEntry->DestRect.Y, 0.0f));

                // Rotation about the center
                Model = glm::translate(Model, glm::vec3(TextureEntry->Center.X, TextureEntry->Center.Y, 0.0f));
                Model = glm::rotate(Model, DegreesToRadians(TextureEntry->Angle), glm::vec3(0.0f, 0.0f, 1.0f));
                Model = glm::translate(Model, glm::vec3(-TextureEntry->Center.X, -TextureEntry->Center.Y, 0.0f));

                // Scale
                Model = glm::scale(Model, glm::vec3(TextureEntry->DestRect.W, TextureEntry->DestRect.H, 1.0f));

                ShaderSetMatrix4f(Shader, "Model", &Model);
                ShaderSetVector4f(Shader, "SpriteColor", &TextureEntry->ColorSpec.Color);
                ShaderSetInteger(Shader, "LuminosityTint", TextureEntry->ColorSpec.Tint == ColorTint_Luminosity);

                glActiveTexture(GL_TEXTURE0);
                BindTexture(Texture);

                glBindVertexArray(TextureRenderer->QuadVAO);
                // Update VBO
                glBindBuffer(GL_ARRAY_BUFFER, TextureRenderer->VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(FrameCoordinates), FrameCoordinates, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Draw
                glDrawArrays(GL_TRIANGLES, 0, 6);

                // Unbind VAO
                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } break;
            case RenderEntry_Text:
            {
                render_entry_text* TextEntry = &Entry->Text;
                text_renderer* TextRenderer = &Renderer->TextRenderer;
#if 0
                // Draw entire atlas
		real32 X = TextEntry->Position.X;
		real32 Y = TextEntry->Position.Y;
                uint32 AtlasWidth = TextRenderer->FontAtlas.Width;
                uint32 AtlasHeight = TextRenderer->FontAtlas.Height;
                GLfloat Vertices[6][4] = {
                    { X,         Y + AtlasHeight, 0.0, 0.0 },
                    { X,         Y,          0.0, 1.0 },
                    { X + AtlasWidth, Y,          1.0, 1.0 },

                    { X,         Y + AtlasHeight, 0.0, 0.0 },
                    { X + AtlasWidth, Y,          1.0, 1.0 },
                    { X + AtlasWidth, Y + AtlasHeight, 1.0, 0.0 }
                };
				glBindVertexArray(TextRenderer->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, TextRenderer->VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);

				glBindTexture(GL_TEXTURE_2D, TextRenderer->FontAtlas.TextureId);
                // Cleanup
                glDrawArrays(GL_TRIANGLES, 0, 6);
#else
                struct glyph_point
                {
                    GLfloat X;
                    GLfloat Y;
                    GLfloat S;
                    GLfloat T;
                };

#if 0
                glyph_point* GlyphCoordinates = new glyph_point[6 * strlen(TextEntry->Text)];
#else
                //glyph_point GlyphCoordinates[6 * strlen(TextEntry->Text)];
                const int32 MaxGlyphCoordinates = 6 * RenderTextLengthMax;
                int32 GlyphCoordinateCount = 6 * (int32)strlen(TextEntry->Text);
                glyph_point GlyphCoordinates[MaxGlyphCoordinates];
#endif

                real32 Scale = 1.0f;
                uint32 N = 0;
                uint32 AtlasWidth = TextRenderer->FontAtlas.Width;
                uint32 AtlasHeight = TextRenderer->FontAtlas.Height;

                real32 CharacterPositionX = TextEntry->Position.X;
                for (uint32 CharacterIndex = 0;
                     CharacterIndex < strlen(TextEntry->Text);
                     ++CharacterIndex)
                {
                    char Character = TextEntry->Text[CharacterIndex];
                    text_character TextCharacter = TextRenderer->FontAtlas.Characters[(int32)Character];

                    GLfloat PositionX = CharacterPositionX + TextCharacter.Bearing.X * Scale;
                    GLfloat PositionY = -TextEntry->Position.Y - (TextCharacter.Size.Y - TextCharacter.Bearing.Y + AtlasHeight) * Scale;

                    CharacterPositionX += (real32)TextCharacter.Advance * Scale;

                    GLfloat Width = TextCharacter.Size.X * Scale;
                    GLfloat Height = TextCharacter.Size.Y * Scale;

                    GLfloat AtlasOffsetX = TextCharacter.AtlasOffsetX;

                    real32 TextureCoordinateLeftX = AtlasOffsetX / (real32)AtlasWidth;
                    real32 TextureCoordinateRightX = (AtlasOffsetX + TextCharacter.Size.X) / (real32)AtlasWidth;

                    GlyphCoordinates[N++] = {
                        PositionX,
                        -PositionY - Height,
                        TextureCoordinateLeftX,
                        0.0f };

                    GlyphCoordinates[N++] = { PositionX,
                        -PositionY,
                        TextureCoordinateLeftX,
                        TextCharacter.Size.Y / AtlasHeight };

                    GlyphCoordinates[N++] = { PositionX + Width,
                        -PositionY,
                        TextureCoordinateRightX,
                        TextCharacter.Size.Y / AtlasHeight };

                    GlyphCoordinates[N++] = { PositionX,
                        -PositionY - Height,
                        TextureCoordinateLeftX,
                        0.0f };

                    GlyphCoordinates[N++] = { PositionX + Width,
                        -PositionY,
                        TextureCoordinateRightX,
                        TextCharacter.Size.Y / AtlasHeight };

                    GlyphCoordinates[N++] = { PositionX + Width,
                        -PositionY - Height,
                        TextureCoordinateRightX,
                        0.0f };
                }


                shader* Shader = TextRenderer->Shader;
                ShaderUse(Shader);
                ShaderSetVector4f(Shader, "TextColor", &TextEntry->Color);

                glActiveTexture(GL_TEXTURE0);

                glBindVertexArray(TextRenderer->VAO);
                glBindTexture(GL_TEXTURE_2D, TextRenderer->FontAtlas.TextureId);
                // Bind VBO here to update it before drawing
                glBindBuffer(GL_ARRAY_BUFFER, TextRenderer->VBO);

                glBufferData(GL_ARRAY_BUFFER, sizeof(glyph_point) * GlyphCoordinateCount, GlyphCoordinates, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDrawArrays(GL_TRIANGLES, 0, N);
#endif

                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);
            } break;
            case RenderEntry_Rectangle:
            {
                render_entry_rectangle* RectangleEntry = &Entry->Rectangle;
                rectangle_renderer* RectangleRenderer = &Renderer->RectangleRenderer;
                // TODO: Support rect outline
                GLfloat HalfSizeX = RectangleEntry->Rect.W / 2.0f;
                GLfloat HalfSizeY = RectangleEntry->Rect.H / 2.0f;
                GLfloat RectData[] = {
                    RectangleEntry->Rect.X + HalfSizeX,
                    RectangleEntry->Rect.Y + HalfSizeY,
                    HalfSizeX,
                    HalfSizeY,
                    RectangleEntry->Color.R,
                    RectangleEntry->Color.G,
                    RectangleEntry->Color.B,
                    RectangleEntry->Color.A
                };

                ShaderUse(RectangleRenderer->Shader);
                glBindVertexArray(RectangleRenderer->VAO);
                // Update VBO
                glBindBuffer(GL_ARRAY_BUFFER, RectangleRenderer->VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(RectData), &RectData, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Draw
                glDrawArrays(GL_POINTS, 0, 4);
                glBindVertexArray(0);
            } break;
            case RenderEntry_Circle:
            {
                render_entry_circle* CircleEntry = &Entry->Circle;
                circle_renderer* CircleRenderer = &Renderer->CircleRenderer;
                GLfloat CircleData[] = {
                    CircleEntry->Position.X,
                    CircleEntry->Position.Y,
                    CircleEntry->Radius,
                    CircleEntry->Color.R,
                    CircleEntry->Color.G,
                    CircleEntry->Color.B,
                    CircleEntry->Color.A
                };

                ShaderUse(CircleRenderer->Shader);
                glBindVertexArray(CircleRenderer->VAO);
                // Update VBO
                glBindBuffer(GL_ARRAY_BUFFER, CircleRenderer->VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(CircleData), &CircleData, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Draw
                glDrawArrays(GL_POINTS, 0, 4);
                glBindVertexArray(0);
            } break;
            case RenderEntry_Polygon4:
            {
                polygon4_renderer* Polygon4Renderer = &Renderer->Polygon4Renderer;
                render_entry_polygon4* Polygon4Entry = &Entry->Polygon4;
                GLfloat Polygon4Data[] = {
                    Polygon4Entry->A.X, Polygon4Entry->A.Y,
                    Polygon4Entry->B.X, Polygon4Entry->B.Y,
                    Polygon4Entry->C.X, Polygon4Entry->C.Y,
                    Polygon4Entry->D.X, Polygon4Entry->D.Y,
                    Polygon4Entry->Color.R,
                    Polygon4Entry->Color.G,
                    Polygon4Entry->Color.B,
                    Polygon4Entry->Color.A
                };

                ShaderUse(Polygon4Renderer->Shader);
                glBindVertexArray(Polygon4Renderer->VAO);
                // Update VBO
                glBindBuffer(GL_ARRAY_BUFFER, Polygon4Renderer->VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Polygon4Data), &Polygon4Data, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Draw
                glDrawArrays(GL_POINTS, 0, 4);
                glBindVertexArray(0);
            } break;
        }
    }

    EndPostProcessor(&Renderer->PostProcessor);
    real32 ScreenShakeMagnitude = GetLargestScreenShakeMagnitude(&Renderer->PostProcessor);
    RenderPostProcessor(&Renderer->PostProcessor, SDL_GetTicks() / 100.0f, ScreenShakeMagnitude / 200.0f);
}

// API
void
AddAnimationSpec(game_state* GameState,
                 animation_spec_id AnimationSpecId,
                 texture_id TextureId,
                 int32 FrameTime)
{
    animation_spec* AnimationSpec = &GameState->Assets.Animations[AnimationSpecId];
    AnimationSpec->TextureId = TextureId;
    AnimationSpec->FrameTime = FrameTime;
    AnimationSpec->FrameCount = 0;
}

void
AddAnimationFrame(game_state* GameState,
                 animation_spec_id AnimationSpecId,
                 rect Rect)
{
    animation_spec* AnimationSpec = &GameState->Assets.Animations[AnimationSpecId];
    AnimationSpec->Frames[AnimationSpec->FrameCount] = Rect;
    ++AnimationSpec->FrameCount;
}
