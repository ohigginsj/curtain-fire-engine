#version 330 core

uniform mat4 Model;
uniform mat4 Projection;
uniform bool FlipX;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 Texture;

out vec2 TexCoords;

void main()
{
    if (FlipX)
    {
        TexCoords = vec2(1.0 - (Texture.x - 0.0), Texture.y);
    }
    else
    {
        TexCoords = Texture;
    }
    gl_Position = Projection * Model * vec4(Position, 1.0);
}
