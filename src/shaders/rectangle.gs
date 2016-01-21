#version 330 core

uniform mat4 Projection;

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in vs_out {
    vec4 Color;
    float HalfSizeX;
    float HalfSizeY;
} GeometryInput[];

out vec4 FragmentColor;

void BuildSquare(vec4 Position)
{
    FragmentColor = GeometryInput[0].Color;
    float HalfSizeX = GeometryInput[0].HalfSizeX;
    float HalfSizeY = GeometryInput[0].HalfSizeY;
    gl_Position = Position + Projection * vec4(-HalfSizeX,  HalfSizeY, 0.0f, 0.0f);
    EmitVertex();
    gl_Position = Position + Projection * vec4( HalfSizeX,  HalfSizeY, 0.0f, 0.0f);
    EmitVertex();
    gl_Position = Position + Projection * vec4(-HalfSizeX, -HalfSizeY, 0.0f, 0.0f);
    EmitVertex();
    gl_Position = Position + Projection * vec4( HalfSizeX, -HalfSizeY, 0.0f, 0.0f);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    BuildSquare(gl_in[0].gl_Position);
}
