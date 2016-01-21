#version 330 core

uniform sampler2D Image;
uniform vec4 SpriteColor;
uniform bool LuminosityTint;

in vec2 TexCoords;
out vec4 Color;

float hue2rgb(float f1, float f2, float hue)
{
    if (hue < 0.0)
        hue += 1.0;
    else if (hue > 1.0)
        hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
        res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
        res = f2;
    else if ((3.0 * hue) < 2.0)
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
        res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl)
{
    vec3 rgb;
    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z); // Luminance
    } else {
        float f2;
        if (hsl.z < 0.5)
            f2 = hsl.z * (1.0 + hsl.y);
        else
            f2 = hsl.z + hsl.y - hsl.y * hsl.z;
        float f1 = 2.0 * hsl.z - f2;
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0/3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0/3.0));
    }
    return rgb;
}

float rgb2hue(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return abs(q.z + (q.w - q.y) / (6.0 * d + e));
}

void main()
{
    vec4 TextureColor = texture(Image, TexCoords);
    // How to premultiplied alpha + luminosity tint?
    /*TextureColor = vec4(TextureColor.rgb * TextureColor.a, TextureColor.a);*/
    if (LuminosityTint &&
        SpriteColor != vec4(1.0, 1.0, 1.0, 1.0))
    {
        float TextureLuminance = 0.2126*TextureColor.r + 0.7152*TextureColor.g + 0.0722*TextureColor.b;
        vec3 AdjustedSpriteColor = hsl2rgb(vec3(rgb2hue(SpriteColor.xyz), 1.0, TextureLuminance));
        Color = vec4(AdjustedSpriteColor, SpriteColor.a * TextureColor.a);
    }
    else
    {
        Color = SpriteColor * TextureColor;
    }
}
