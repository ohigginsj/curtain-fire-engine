#version 330 core

layout (location = 0) in vec2 A;
layout (location = 1) in vec2 B;
layout (location = 2) in vec2 C;
layout (location = 3) in vec2 D;
layout (location = 4) in vec4 Color;

out vs_out {
    vec2 A;
    vec2 B;
    vec2 C;
    vec2 D;
    vec4 Color;
} Vertex_Output;

void main()
{
    Vertex_Output.A = A;
    Vertex_Output.B = B;
    Vertex_Output.C = C;
    Vertex_Output.D = D;
    Vertex_Output.Color = Color;
}
