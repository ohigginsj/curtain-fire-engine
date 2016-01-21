#version 330 core

layout (location = 0) in vec4 Vertex; // <vec2 Position, vec2 TextureCoordinates>

out vec2 TextureCoordinates;

uniform mat4 Projection;
uniform float ShakeMagnitude;
uniform float Time;

void main()
{
    gl_Position = Projection * vec4(Vertex.xy, 0.0, 1.0);
    TextureCoordinates = Vertex.zw;
    vec2 InputTextureCoordinates = Vertex.zw;
    if (ShakeMagnitude > 0.0)
    {
        gl_Position.x += cos(Time * 10) * ShakeMagnitude;
        gl_Position.y += cos(Time * 15) * ShakeMagnitude;
    }
}
