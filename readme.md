# Curtain Fire Engine v0.1

## Build

### Dependencies:
- OpenGL 3.3
- [SDL2](https://www.libsdl.org/)
- [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)
- [glad](https://github.com/Dav1dde/glad)
- [glm](http://glm.g-truc.net/)
- [FreeType](http://www.freetype.org/)
- [stb_image](https://github.com/nothings/stb)

### Linux (LLVM)
1. Install SDL2 and SDL2_mixer development libraries
2. Install Freetype
1. Run build.sh.
2. Run run.sh

Note: g++ also works although the warning options in build.sh will differ.

### Windows (MSVC)
1. Open a command prompt
1. Load the Windows dev environment (vcvarsall.bat)
2. Run build.bat
3. Run run.bat

## Engine Features
- Live code editing
- API for creating animations, ships, weapons, stages, and bosses
- 3 projectile types: Bullets, Loose Lasers, Straight lasers
- Tint grayscale sprites while preserving luminosity, allowing for any color of projectile from a single art asset.
- Basic bombs
- Animated particles
- Circular + Rectangular collision detection (with arbitrary rotations)
- Projectile grazing
- Debug view with collision data display
- TTF font rendering

## Demo

### Features
- One player ship with 2 weapons
- One boss with 3 attacks

### Credits
- Boss attack patterns ported from [Spell Card Collection](http://www.bulletforge.org/u/shijimi_nono/p/spellcardcollection) (danmakufu script)
- Bullet graphics and sounds from [danmakufu](http://www.geocities.co.jp/SiliconValley-Oakland/9951/pre/th_dnh_ph3.html) (PC)
- Boss graphic from Super Robot Wars (SNES)
- Hind graphic from Desert War (Arcade)
- Explosion graphics from Strikers (Arcade)
