#version 330 core

const float PI = 3.1415926;

uniform mat4 Projection;

layout (points) in;
layout (line_strip, max_vertices = 64) out;

in vs_out {
    vec4 Color;
    float Radius;
} GeometryInput[];

out vec4 FragmentColor;

void BuildCircle(vec4 Position)
{
    FragmentColor = GeometryInput[0].Color;
    float Radius = GeometryInput[0].Radius;
    int Sides = 32;
    for (int VertexIndex = 0;
         VertexIndex <= Sides + 1;
         ++VertexIndex)
    {
        float Angle = PI * 2.0 / Sides * VertexIndex;

        vec4 Offset = vec4(cos(Angle) * Radius, -sin(Angle) * Radius, 0.0, 0.0);
        gl_Position = Position + Projection * Offset;
        EmitVertex();
    }
}

void main()
{
    BuildCircle(gl_in[0].gl_Position);
}
