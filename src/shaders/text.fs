#version 330 core

in vec2 TexCoords;
out vec4 Color;

uniform sampler2D Text;
uniform vec4 TextColor;

void main()
{
    vec4 Sampled = vec4(1.0, 1.0, 1.0, texture(Text, TexCoords).r);
    Color = TextColor * Sampled;
    // Premultiplied alpha!!
    Color.x = Color.x * Color.w;
    Color.y = Color.y * Color.w;
    Color.z = Color.z * Color.w;
}
