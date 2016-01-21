#version 330 core

uniform mat4 Projection;

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 HalfSize;
layout (location = 2) in vec4 Color;

out vs_out {
    vec4 Color;
    float HalfSizeX;
    float HalfSizeY;
} Vertex_Output;

void main()
{
    gl_Position = Projection * vec4(Position.x, Position.y, 0.0, 1.0);
    Vertex_Output.Color = Color;
    Vertex_Output.HalfSizeX = HalfSize.x;
    Vertex_Output.HalfSizeY = HalfSize.y;
}
