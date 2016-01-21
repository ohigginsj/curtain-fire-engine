#version 330 core

in vec2 TextureCoordinates;
out vec4 Color;

uniform sampler2D Scene;
uniform vec2 Offsets[9];
uniform float BlurKernel[9];

uniform float ShakeMagnitude;

void main()
{
    // Blur effect
    /*if (ShakeMagnitude > 0.0)*/
    /*{*/
        /*for (int CoordinateIndex = 0;*/
             /*CoordinateIndex < 9;*/
             /*++CoordinateIndex)*/
        /*{*/
            /*vec3 Sample = vec3(texture(Scene, TextureCoordinates.st + Offsets[CoordinateIndex]));*/
            /*Color += vec4(Sample * BlurKernel[CoordinateIndex], 0.0);*/
            /*[>Color += vec4(Sample, 0.0);<]*/
        /*}*/
        /*Color.a = 1.0;*/
    /*}*/
    /*else*/
    {
        Color = texture(Scene, TextureCoordinates);
    }
}
