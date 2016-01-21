#version 330 core

uniform mat4 Projection;

layout (points) in;
layout (line_strip, max_vertices = 5) out;

in vs_out {
    vec2 A;
    vec2 B;
    vec2 C;
    vec2 D;
    vec4 Color;
} GeometryInput[];

out vec4 FragmentColor;

void BuildPolygon4()
{
    FragmentColor = GeometryInput[0].Color;
    gl_Position = Projection * vec4(GeometryInput[0].A.xy, 0.0, 1.0);
    EmitVertex();
    gl_Position = Projection * vec4(GeometryInput[0].B.xy, 0.0, 1.0);
    EmitVertex();
    gl_Position = Projection * vec4(GeometryInput[0].C.xy, 0.0, 1.0);
    EmitVertex();
    gl_Position = Projection * vec4(GeometryInput[0].D.xy, 0.0, 1.0);
    EmitVertex();
    gl_Position = Projection * vec4(GeometryInput[0].A.xy, 0.0, 1.0);
    EmitVertex();
}

void main()
{
    BuildPolygon4();
}
