#include "api.h"

// Layers
const int32 Layer_LaserDefault = 55;
const int32 Layer_ProjectileDefault = 50;

// Hind
const real32 Hind_MoveAcceleration_Normal = 8.0f;
const real32 Hind_MoveAcceleration_Focus = 4.0f;
const real32 Hind_MoveSpeedMax_Normal = 4.0f;
const real32 Hind_MoveSpeedMax_Focus = 1.6f;
const v2     Hind_BoundsCollisionSize = V2(10.0f, 20.0f);
const real32 Hind_CollisionAreaRadius = 3.0f;
const real32 Hind_GrazeCollisionAreaRadius = 12.0f;
const real32 Hind_FatalCollisionTime = 24;
const int32  Hind_BombMeterMax = 6;
const int32  Hind_BombCountMax = 3;
const int32  Hind_BombDuration = 30;
const real32 Hind_BombGrowthRate = 5.0f;
const v2     Hind_RenderOffset = V2(0.0f, 5.0f);
const sprite_spec Hind_ShipSprites[] = {
    Sprite(TextureId_Hind_Body, V2(0.0f, 0.0f)),
    Sprite_Animated(AnimationSpecId_Propeller, V2(0.0f, -5.0f))
};
const sprite_spec Hind_ShipFocusSprites[] = {
    Sprite(TextureId_Hind_Body, V2(0.0f, 0.0f)),
    Sprite_Animated(AnimationSpecId_Propeller, V2(0.0f, -5.0f)),
    Sprite_Animated(AnimationSpecId_Propeller_Tracer, V2(0.0f, -5.0f))
};

// Machine Gun
const real32 MachineGun_FireCooldown = 0.05f;
const real32 MachineGun_FireSpeed = 10.0f;
const real32 MachineGun_Inaccuracy = 5.0f;
const v2     MachineGun_ShotOffsets[] = {
    V2(-6.0f, -12.0f),
    V2(6.0f, -12.0f)
};
const real32 MachineGun_FireKickback = 2.0f;
const real32 MachineGun_FireKickbackReturnTime = 0.3f;
const real32 MachineGun_DelayReturnSpeed = 100.0f;
const v2     MachineGun_DelayMaxDistance = V2(2.0f, 2.0f);
const projectile_sprite_id MachineGun_ProjectileSpriteId = ProjectileSprite_Rice_S;

// Minigun
const real32 Minigun_FireCooldown = 0.03f;
const real32 Minigun_FireSpeed = 10.0f;
const real32 Minigun_Inaccuracy = 10.0f;
const v2     Minigun_ShotOffsets[] = {
    V2(-12.0f, -8.0f),
    V2(12.0f, -8.0f)
};
const real32 Minigun_FireKickback = 1.0f;
const real32 Minigun_FireKickbackReturnTime = 0.3f;
const real32 Minigun_DelayReturnSpeed = 100.0f;
const v2     Minigun_DelayMaxDistance = V2(1.0f, 1.0f);
const projectile_sprite_id Minigun_ProjectileSpriteId = ProjectileSprite_Rice_S;

void
DefineGame(game_state* GameState)
{
    SetUpdateFunction(GameState, UpdateStage1);

    DefineShip(GameState, ShipId_Hind,
               Hind_MoveAcceleration_Normal,
               Hind_MoveAcceleration_Focus,
               Hind_MoveSpeedMax_Normal,
               Hind_MoveSpeedMax_Focus,
               Hind_BoundsCollisionSize,
               Hind_CollisionAreaRadius,
               Hind_GrazeCollisionAreaRadius,
               Hind_FatalCollisionTime,
               Hind_BombMeterMax,
               Hind_BombCountMax,
               Hind_BombDuration,
               Hind_BombGrowthRate,
               Hind_RenderOffset,
               Hind_ShipSprites,
               ArrayCount(Hind_ShipSprites),
               Hind_ShipFocusSprites,
               ArrayCount(Hind_ShipFocusSprites));
    DefineWeapon(GameState, WeaponId_MachineGun,
                 MachineGun_FireCooldown,
                 MachineGun_FireSpeed,
                 MachineGun_Inaccuracy,
                 MachineGun_ProjectileSpriteId,
                 MachineGun_ShotOffsets,
                 ArrayCount(MachineGun_ShotOffsets),
                 MachineGun_FireKickback,
                 MachineGun_FireKickbackReturnTime,
                 MachineGun_DelayReturnSpeed,
                 MachineGun_DelayMaxDistance,
                 TextureId_Weapon_MachineGun);
    DefineWeapon(GameState, WeaponId_Minigun,
                 Minigun_FireCooldown,
                 Minigun_FireSpeed,
                 Minigun_Inaccuracy,
                 Minigun_ProjectileSpriteId,
                 Minigun_ShotOffsets,
                 ArrayCount(Minigun_ShotOffsets),
                 Minigun_FireKickback,
                 Minigun_FireKickbackReturnTime,
                 Minigun_DelayReturnSpeed,
                 Minigun_DelayMaxDistance,
                 TextureId_Weapon_Minigun);

    SetPlayerShipId(GameState, ShipId_Hind);
    SetPlayerWeaponId_Normal(GameState, WeaponId_Minigun);
    SetPlayerWeaponId_Focus(GameState, WeaponId_MachineGun);

    // Sounds
    LoadSound(GameState, SoundId_ShotA, "data/sounds/effects/shot1.wav");
    LoadSound(GameState, SoundId_ShotB, "data/sounds/effects/shot2.wav");
    LoadSound(GameState, SoundId_ShotC, "data/sounds/effects/shot3.wav");
    LoadSound(GameState, SoundId_ShotD, "data/sounds/effects/shot4.wav");
    LoadSound(GameState, SoundId_ShotE, "data/sounds/effects/shot5.wav");
    LoadSound(GameState, SoundId_Freeze, "data/sounds/effects/freeze.wav");
    LoadSound(GameState, SoundId_LaserA, "data/sounds/effects/laser1.wav");
    LoadSound(GameState, SoundId_LaserB, "data/sounds/effects/laser2.wav");
    LoadSound(GameState, SoundId_LaserC, "data/sounds/effects/laser3.wav");
    LoadSound(GameState, SoundId_ChargeA, "data/sounds/effects/charge1.wav");
    LoadSound(GameState, SoundId_ChargeB, "data/sounds/effects/charge2.wav");
    LoadSound(GameState, SoundId_ChargeC, "data/sounds/effects/charge3.wav");
    LoadSound(GameState, SoundId_Graze, "data/sounds/effects/graze.wav");

    //AddSoundGroup(GameState, SoundGroupId_Shoot, SoundId_ShotA, 5);
    //AddSoundGroup(GameState, SoundGroupId_Impact, SoundId_ImpactA, 2);

    // Textures
    LoadTexture(GameState, TextureId_Background,        "data/graphics/bg_circuit_board.png", TextureFilter_Linear);
    LoadTexture(GameState, TextureId_Weapon_MachineGun, "data/graphics/hind_machine_gun.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Weapon_Minigun,    "data/graphics/hind_minigun.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Enemy_Scarab,      "data/graphics/impetus.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Hind_Body,         "data/graphics/hind_body.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Hind_Propeller,    "data/graphics/hind_propeller.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_BulletMap,         "data/graphics/bullets.png", TextureFilter_Linear);
    LoadTexture(GameState, TextureId_MuzzleFlash,       "data/graphics/particles/muzzle_flash.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Particle_Smoke,    "data/graphics/particles/smoke.png", TextureFilter_Linear);
    LoadTexture(GameState, TextureId_Explosion_0,       "data/graphics/particles/explosion_0.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Explosion_1,       "data/graphics/particles/explosion_1.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Explosion_2,       "data/graphics/particles/explosion_2.png", TextureFilter_Nearest);
    LoadTexture(GameState, TextureId_Flare,             "data/graphics/particles/graze.png", TextureFilter_Linear);

    // Background
    LoadScrollingBackground(GameState, TextureId_Background, 88, 200.0f);

    // Animations
    AddAnimationSpec(GameState, AnimationSpecId_Propeller, TextureId_Hind_Propeller, 1);
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(0 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(1 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(2 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(3 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(4 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(5 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(6 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(7 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(8 * 64, 0, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller, Rect(9 * 64, 0, 64, 54));

    AddAnimationSpec(GameState, AnimationSpecId_Propeller_Tracer, TextureId_Hind_Propeller, 2);
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(0 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(1 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(2 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(3 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(4 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(5 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(6 * 64, 54, 64, 54));
    AddAnimationFrame(GameState, AnimationSpecId_Propeller_Tracer, Rect(7 * 64, 54, 64, 54));

    AddAnimationSpec(GameState, AnimationSpecId_Explosion_0, TextureId_Explosion_0, 2);
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(0,   0, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(129, 0, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(258, 0, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(387, 0, 128, 104));

    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(0,   105, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(129, 105, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(258, 105, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(387, 105, 128, 104));

    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(0,   210, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(129, 210, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(258, 210, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(387, 210, 128, 104));

    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(0,   315, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(129, 315, 128, 104));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_0, Rect(258, 315, 128, 104));

    AddAnimationSpec(GameState, AnimationSpecId_Explosion_1, TextureId_Explosion_1, 1);
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(0 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(1 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(2 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(3 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(4 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(5 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(6 * 65, 0, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(7 * 65, 0, 64, 64));

    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(0 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(1 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(2 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(3 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(4 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(5 * 65, 66, 64, 64));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_1, Rect(6 * 65, 66, 64, 64));

    AddAnimationSpec(GameState, AnimationSpecId_Explosion_2, TextureId_Explosion_2, 2);
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(0 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(1 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(2 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(3 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(4 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(5 * 17, 0, 16, 48));
    AddAnimationFrame(GameState, AnimationSpecId_Explosion_2, Rect(6 * 17, 0, 16, 48));

    // Bullet Specs
    DefineBulletSprite(GameState, ProjectileSprite_Ball_SS, Rect(1, 1, 9, 10), 2.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_S, Rect(1, 12, 15, 15), 4.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_M, Rect(28, 78, 27, 27), 8.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_M_Add, Rect(28, 78, 27, 27), 8.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Add);
    DefineBulletSprite(GameState, ProjectileSprite_Needle, Rect(29, 28, 7, 17), 2.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Rice_S, Rect(83, 29, 9, 16), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ice, Rect(37, 28, 9, 17), 2.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Bullet, Rect(1, 28, 9, 18), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Rice_M, Rect(63, 1, 15, 26), 6.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Kunai, Rect(79, 1, 11, 20), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Scale, Rect(108, 29, 15, 14), 4.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Bill, Rect(93, 29, 14, 15), 5.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Star_S, Rect(47, 28, 17, 17), 7.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Star_M, Rect(22, 46, 28, 29), 10.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Laser, Rect(1, 78, 26, 28), 0.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Add);
    DefineBulletSprite(GameState, ProjectileSprite_Coin, Rect(65, 29, 17, 16), 4.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_Ring, Rect(11, 28, 17, 17), 3.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_Glow, Rect(38, 1, 24, 25), 3.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Add);
    DefineBulletSprite(GameState, ProjectileSprite_Butterfly, Rect(91, 1, 29, 27), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Dagger_Youmu, Rect(17, 1, 20, 26), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Dagger_Kouma, Rect(1, 47, 20, 30), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Ball_L, Rect(56, 46, 62, 62), 3.0f, Directionality_Angle, ColorTint_Luminosity, BlendMode_Blend);
    DefineBulletSprite(GameState, ProjectileSprite_Delay, Rect(0, 107, 30, 31), 0.0f, Directionality_None, ColorTint_Luminosity, BlendMode_Add);
}
