# Roadmap

## Functionality
- boss animations
- boss attack transitions
- local multiplayer
- Arbitrary viewport size
- Slow motion
- 3D rendering

## Architecture
- Better scheme for persistent projectile/boss data. Ideally allows for arbitrary storage in userland.
- Make a matrix library; get rid of glm
- Unify circular and rectangular collision info
- Release build

## Bugs
- Objects may linger visually for one frame after they are destroyed, probably because they are destroyed after they've submitted their render entries.
- Projectiles are sometimes created after the screen is cleared
- Decreasing alpha makes the luminosity tinting go to red. Probably because hue 0 is red.

## Optimization
- OpenGL instanced rendering
- Geometry renderer is slow

## Polish
- controller support (SDL_GameController doesn't recognize Sixaxis?)
- recalibrate screen shake values now that its in a shader
- [demo] consolidate graphics into atlases

## Not sure
- curvy lasers
- scripting language for user side (lua or squirrel?)
