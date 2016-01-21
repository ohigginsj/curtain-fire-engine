#version 330 core

in vec4 FragmentColor;
out vec4 Color;

void main()
{
    Color = vec4(FragmentColor);
}
