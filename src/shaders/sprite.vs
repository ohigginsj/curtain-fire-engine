#version 330 core

uniform mat4 Model;
uniform mat4 Projection;
uniform bool FlipX;

layout (location = 0) in vec4 Vertex; // <vec2 Position, vec2 TexCoords>

out vec2 TexCoords;

void main()
{
    vec2 Texture = Vertex.zw;
    if (FlipX)
    {
        TexCoords = vec2(1.0 - (Texture.x - 0.0), Texture.y);
    }
    else
    {
        TexCoords = Texture;
    }
    gl_Position = Projection * Model * vec4(Vertex.xy, 0.0, 1.0);
}
