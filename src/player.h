#pragma once

#include "entity.h"
#include "render.h"
#include "particle.h"

enum ship_control_mode
{
    ShipControlMode_Normal,
    ShipControlMode_Focus,
};

struct bomb
{
    bool32 Active;
    v2 Origin;
    int32 Timer;
    int32 Duration;
};

const uint32 PlayerWeaponShotCountMax = 16;
const uint32 PlayerSpriteCountMax = 16;

struct weapon
{
    // Permanent
    uint32 ShotCount;
    v2 ShotOffsets[PlayerWeaponShotCountMax];

    real32 FireCooldown;
    real32 FireSpeed;
    real32 Inaccuracy;
    real32 FireKickback;
    real32 FireKickbackReturnTime;
    real32 DelayReturnSpeed;
    v2 DelayMaxDistance;

    texture_id TextureId;
    projectile_sprite_id ProjectileSpriteId;

    // Transient
    real32 FireCooldownRemaining;
    v2 Offset_Delay;
    v2 Offset_Kickback;
};

struct bomb_module
{
    // Permanent
    int32 MeterMax;
    int32 BombCountMax;
    int32 BombDuration;
    real32 BombGrowthRate;

    // Transient
    int32 BombCount;
    int32 MeterValue;
};

struct ship
{
    // Permanent
    // NOTE: Sprites are assumed to be ordered from furthest to nearest
    uint32 SpriteCount;
    sprite Sprites[PlayerSpriteCountMax];
    uint32 FocusSpriteCount;
    sprite FocusSprites[PlayerSpriteCountMax];

    real32 MoveAcceleration_Normal;
    real32 MoveAcceleration_Focus;
    real32 MoveSpeedMax_Normal;
    real32 MoveSpeedMax_Focus;
    v2 BoundsCollisionSize;
    real32 FatalCollisionTime;
    v2 RenderOffset;

    weapon_id WeaponId_Normal;
    weapon_id WeaponId_Focus;

    // Transient
    bool32 Dead;
    bool32 Firing;
    ship_control_mode ControlMode;
    spatial Spatial;
    collision_area_circle DamageCollisionArea;
    collision_area_circle GrazeCollisionArea;
    bomb_module BombModule;
};

struct player
{
    ship_id ShipId;

    // Keybinds, score, rank, etc.
};

void
InitializeWeapon(game_mode_world* World, weapon_id WeaponId)
{
    weapon* Weapon = &World->Weapons[WeaponId];
    Weapon->FireCooldownRemaining = 0.0f;
    Weapon->Offset_Delay = V2();
    Weapon->Offset_Kickback = V2();
}

void
InitializeShip(game_mode_world* World, ship_id ShipId)
{
    ship* Ship = &World->Ships[ShipId];
    Ship->Dead = false;
    Ship->Firing = false;
    Ship->ControlMode = ShipControlMode_Normal;
    Ship->Spatial.Position = V2(WorldSizeX / 2.0f, WorldSizeY - 64);
    Ship->DamageCollisionArea.Collidable = true;
    Ship->BombModule.BombCount = 1;
    Ship->BombModule.MeterValue = 0.0f;

    for (uint32 SpriteIndex = 0;
         SpriteIndex < Ship->SpriteCount;
         ++SpriteIndex)
    {
        sprite* Sprite = &Ship->Sprites[SpriteIndex];
        if (Sprite->Type == SpriteType_Animation)
        {
            Sprite->Animation.FrameIndex = 0;
        }
    }

    InitializeWeapon(World, Ship->WeaponId_Normal);
    InitializeWeapon(World, Ship->WeaponId_Focus);
}

void
CopySpritesFromSpec(sprite* DestSprites, const sprite_spec* Sprites, uint32 SpriteCount)
{
    Assert("Too many sprites.", SpriteCount < PlayerSpriteCountMax);
    for (uint32 SpriteIndex = 0;
         SpriteIndex < SpriteCount;
         ++SpriteIndex)
    {
        sprite* DestSprite = &DestSprites[SpriteIndex];
        const sprite_spec* SourceSprite = &Sprites[SpriteIndex];
        DestSprite->RenderOffset = SourceSprite->RenderOffset;
        DestSprite->Type = SourceSprite->Type;
        if (SourceSprite->Type == SpriteType_Static)
        {
            DestSprite->TextureId = SourceSprite->TextureId;
        }
        else if (SourceSprite->Type == SpriteType_Animation)
        {
            DestSprite->Animation.SpecId = SourceSprite->AnimationSpecId;
            DestSprite->Animation.FrameIndex = -1;
            DestSprite->Animation.Complete = false;
        }
    }
}

// API
void
DefineShip(game_state* GameState,
           ship_id ShipId,
           real32 MoveAcceleration_Normal,
           real32 MoveAcceleration_Focus,
           real32 MoveSpeedMax_Normal,
           real32 MoveSpeedMax_Focus,
           v2 BoundsCollisionSize,
           real32 DamageCollisionAreaRadius,
           real32 GrazeCollisionAreaRadius,
           real32 FatalCollisionTime,
           int32 BombMeterMax,
           int32 BombCountMax,
           int32 BombDuration,
           real32 BombGrowthRate,
           v2 RenderOffset,
           const sprite_spec* Sprites,
           uint32 SpriteCount,
           const sprite_spec* FocusSprites,
           uint32 FocusSpriteCount)
{
    ship* Ship = &GameState->World->Ships[ShipId];
    Ship->MoveAcceleration_Normal = MoveAcceleration_Normal;
    Ship->MoveAcceleration_Focus = MoveAcceleration_Focus;
    Ship->MoveSpeedMax_Normal = MoveSpeedMax_Normal;
    Ship->MoveSpeedMax_Focus = MoveSpeedMax_Focus;
    Ship->BoundsCollisionSize = BoundsCollisionSize;
    Ship->DamageCollisionArea.Radius = DamageCollisionAreaRadius;
    Ship->GrazeCollisionArea.Radius = GrazeCollisionAreaRadius;
    Ship->FatalCollisionTime = FatalCollisionTime;
    Ship->BombModule.MeterMax = BombMeterMax;
    Ship->BombModule.BombCountMax = BombCountMax;
    Ship->BombModule.BombDuration = BombDuration;
    Ship->BombModule.BombGrowthRate = BombGrowthRate;
    Ship->RenderOffset = RenderOffset;

    CopySpritesFromSpec(Ship->Sprites, Sprites, SpriteCount);
    Ship->SpriteCount = SpriteCount;

    CopySpritesFromSpec(Ship->FocusSprites, FocusSprites, FocusSpriteCount);
    Ship->FocusSpriteCount = FocusSpriteCount;

}

void
DefineWeapon(game_state* GameState,
             weapon_id WeaponId,
             real32 FireCooldown,
             real32 FireSpeed,
             real32 Inaccuracy,
             projectile_sprite_id ProjectileSpriteId,
             const v2* ShotOffsets,
             uint32 ShotCount,
             real32 FireKickback,
             real32 FireKickbackReturnTime,
             real32 DelayReturnSpeed,
             v2 DelayMaxDistance,
             texture_id TextureId)
{
    game_mode_world* World = GameState->World;
    weapon* Weapon = &World->Weapons[WeaponId];
    Weapon->FireCooldown = FireCooldown;
    Weapon->FireSpeed = FireSpeed;
    Weapon->Inaccuracy = Inaccuracy;
    Weapon->ProjectileSpriteId = ProjectileSpriteId;
    Weapon->ShotCount = ShotCount;
    memcpy(Weapon->ShotOffsets, ShotOffsets, ShotCount * sizeof(v2));
    Weapon->FireKickback = FireKickback;
    Weapon->FireKickbackReturnTime = FireKickbackReturnTime;
    Weapon->DelayReturnSpeed = DelayReturnSpeed;
    Weapon->DelayMaxDistance = DelayMaxDistance;
    Weapon->TextureId = TextureId;
    InitializeWeapon(World, WeaponId);
}

void
SetPlayerShipId(game_state* GameState,
                ship_id ShipId)
{
    GameState->World->Player->ShipId = ShipId;
}

void
SetPlayerWeaponId_Normal(game_state* GameState,
                         weapon_id WeaponId)
{
    game_mode_world* World = GameState->World;
    ship* Ship = &World->Ships[World->Player->ShipId];
    Ship->WeaponId_Normal = WeaponId;
}

void
SetPlayerWeaponId_Focus(game_state* GameState,
                        weapon_id WeaponId)
{
    game_mode_world* World = GameState->World;
    ship* Ship = &World->Ships[World->Player->ShipId];
    Ship->WeaponId_Focus = WeaponId;
}

void
InitializePlayer(game_mode_world* World)
{
    player* Player = World->Player;
    InitializeShip(World, Player->ShipId);
}

inline v2
GetPlayerPosition(game_state* GameState)
{
    game_mode_world* World = GameState->World;
    v2 Result = V2(0.0f, 0.0f);

    ship* Ship = &World->Ships[World->Player->ShipId];
    if (Ship->Dead)
    {
        Result = V2(WorldSizeX / 2.0f, WorldSizeY - 50.0f);
    }
    else
    {
        Result = Ship->Spatial.Position;
    }

    return Result;
}

real32
GetAngleToPlayer(game_state* GameState, v2 Position)
{
    v2 Direction = GetPlayerPosition(GameState) - Position;
    return VectorAngle(Direction);
}

void
UseBomb(game_mode_world* World)
{
    ship* Ship = &World->Ships[World->Player->ShipId];
    bomb_module* BombModule = &Ship->BombModule;
    if (BombModule->BombCount > 0)
    {
        bomb* Bomb = World->Bomb;
        Bomb->Active = true;
        Bomb->Timer = 0;
        Bomb->Duration = BombModule->BombDuration;
        Bomb->Origin = Ship->Spatial.Position;

        --BombModule->BombCount;

        if (World->ControlMode == ControlMode_FatalCollision)
        {
            BombModule->MeterValue = 0.0f;
        }
    }

    World->ControlMode = ControlMode_Normal;
}

real32
GetBombRadius(game_mode_world* World)
{
    ship* Ship = &World->Ships[World->Player->ShipId];
    bomb_module* BombModule = &Ship->BombModule;
    real32 Result = Ship->DamageCollisionArea.Radius + ((real32)World->Bomb->Timer * BombModule->BombGrowthRate);
    return Result;
}

void
IncrementBombMeter(bomb_module* BombModule)
{
    if (BombModule->BombCount < BombModule->BombCountMax)
    {
        BombModule->MeterValue += 1;
        if (BombModule->MeterValue == BombModule->MeterMax)
        {
            BombModule->BombCount += 1;
            BombModule->MeterValue = 0;
        }
    }
}

void
KillShip(game_state* GameState)
{
    game_mode_world* World = GameState->World;
    ship* Ship = &World->Ships[World->Player->ShipId];
    spatial* ShipSpatial = &Ship->Spatial;

    ScreenShake(GameState->Renderer, 3.0f, 0.4f);
    CreateParticle_Animated(World,
                            ShipSpatial->Position,
                            V2(0.0f, 0.0f),
                            15, 1.0f,
                            ColorSpec_None,
                            50,
                            AnimationSpecId_Explosion_0);

    Ship->Dead = true;
    World->ControlMode = ControlMode_DeathAnimation;
    World->DeathAnimationTime = 40;
}

void
RenderSpriteArray(game_state* GameState,
                  sprite* Sprites, uint32 Count,
                  v2 Position,
                  int32 Layer)
{
    for (uint32 SpriteIndex = 0;
         SpriteIndex < Count;
         ++SpriteIndex)
    {
        sprite* Sprite = &Sprites[SpriteIndex];
        v2 TotalRenderOffset = Position + Sprite->RenderOffset;
        if (Sprite->Type == SpriteType_Static)
        {
            PushTexture(GameState->Renderer,
                        Sprite->TextureId,
                        GetFullTextureRect(GameState, Sprite->TextureId),
                        TotalRenderOffset,
                        1.0f, Layer);
        }
        else if (Sprite->Type == SpriteType_Animation)
        {
            PushAnimation(GameState,
                          &Sprite->Animation,
                          TotalRenderOffset,
                          1.0f, V2(0.0f, 0.0f), 0.0f,
                          ColorSpec_None,
                          Layer, BlendMode_Blend);
        }
    }
}

void
UpdateSpriteArray(game_state* GameState, sprite* Sprites, uint32 Count)
{
    for (uint32 SpriteIndex = 0;
         SpriteIndex < Count;
         ++SpriteIndex)
    {
        sprite* Sprite = &Sprites[SpriteIndex];
        if (Sprite->Type == SpriteType_Animation)
        {
            UpdateAnimation(GameState, &Sprite->Animation);
        }
    }
}

void
UpdateWeapon(weapon* Weapon, v2 ShipVelocity, real32 DeltaTime)
{
    if (Weapon->Offset_Kickback.Y > 0.0f)
    {
        real32 FireKickbackReturnSpeed = Weapon->FireKickback / Weapon->FireKickbackReturnTime;
        Weapon->Offset_Kickback.Y -= FireKickbackReturnSpeed * DeltaTime;
    }
    if (Weapon->Offset_Kickback.Y < 0.0f)
    {
        Weapon->Offset_Kickback.Y = 0.0f;
    }

    Weapon->Offset_Delay -= ShipVelocity;
    real32 FrameDelayReturnSpeed = Weapon->DelayReturnSpeed * DeltaTime;
    if (Weapon->Offset_Delay.Y > FrameDelayReturnSpeed)
    {
        Weapon->Offset_Delay.Y -= FrameDelayReturnSpeed;
    }
    else if (Weapon->Offset_Delay.Y < -FrameDelayReturnSpeed)
    {
        Weapon->Offset_Delay.Y += FrameDelayReturnSpeed;
    }
    else
    {
        Weapon->Offset_Delay.Y = 0.0f;
    }
    Clamp(&Weapon->Offset_Delay.Y,
          -Weapon->DelayMaxDistance.Y,
          Weapon->DelayMaxDistance.Y);

    if (Weapon->Offset_Delay.X > FrameDelayReturnSpeed)
    {
        Weapon->Offset_Delay.X -= FrameDelayReturnSpeed;
    }
    else if (Weapon->Offset_Delay.X < -FrameDelayReturnSpeed)
    {
        Weapon->Offset_Delay.X += FrameDelayReturnSpeed;
    }
    else
    {
        Weapon->Offset_Delay.X = 0.0f;
    }
    Clamp(&Weapon->Offset_Delay.X,
          -Weapon->DelayMaxDistance.X,
          Weapon->DelayMaxDistance.X);
}

void
RenderWeapon(game_state* GameState, weapon* Weapon, v2 Position, int32 Layer)
{
    PushTexture(GameState->Renderer,
                Weapon->TextureId,
                GetFullTextureRect(GameState, Weapon->TextureId),
                Position +
                    Weapon->Offset_Kickback +
                    Weapon->Offset_Delay,
                1.0f, Layer);
}

void
UpdatePlayer(game_state* GameState,
             game_input* Input)
{
    game_mode_world* World = GameState->World;
    player* Player = World->Player;
    ship* Ship = &World->Ships[World->Player->ShipId];
    spatial* ShipSpatial = &Ship->Spatial;
    collision_area_circle* ShipCollision = &Ship->DamageCollisionArea;
    int32 PlayerLayer = EngineLayer_PlayerDefault;

    if (Ship->Dead)
    {
        return;
    }

    Ship->ControlMode = Input->Shift.Down ?
        ShipControlMode_Focus :
        ShipControlMode_Normal;

    real32 MoveAcceleration = Ship->ControlMode == ShipControlMode_Focus ?
        Ship->MoveAcceleration_Focus :
        Ship->MoveAcceleration_Normal;

    real32 MoveSpeedMax = Ship->ControlMode == ShipControlMode_Focus ?
        Ship->MoveSpeedMax_Focus :
        Ship->MoveSpeedMax_Normal;

    bool32 MovingX = false;
    bool32 MovingY = false;
    v2 MoveSpec = V2(0.0f, 0.0f);
    if (Input->ArrowRight.Down)
    {
        MoveSpec.X += MoveAcceleration;
        MovingX = true;
    }
    if (Input->ArrowLeft.Down)
    {
        MoveSpec.X += -MoveAcceleration;
        MovingX = true;
    }
    if (Input->ArrowUp.Down)
    {
        MoveSpec.Y += -MoveAcceleration;
        MovingY = true;
    }
    if (Input->ArrowDown.Down)
    {
        MoveSpec.Y += MoveAcceleration;
        MovingY = true;
    }

    v2 TentativeVelocity = ShipSpatial->Velocity + MoveSpec;
    if (Magnitude(TentativeVelocity) > MoveSpeedMax)
    {
        TentativeVelocity = Rescale(TentativeVelocity, MoveSpeedMax);
    }
    ShipSpatial->Velocity = TentativeVelocity;

    // NOTE: Instant stop for now
    if(!MovingX)
    {
        ShipSpatial->Velocity.X = 0;
    }
    if (!MovingY)
    {
        ShipSpatial->Velocity.Y = 0;
    }

    UpdateSpatial(ShipSpatial);

    real32 Radius = ShipCollision->Radius;
    v2 LimitMin = V2(Radius,
                     Radius);
    v2 LimitMax = V2(WorldSizeX - Radius,
                     WorldSizeY - Radius);
    if (ShipSpatial->Position.X < LimitMin.X + Ship->BoundsCollisionSize.X)
    {
        ShipSpatial->Position.X = LimitMin.X + Ship->BoundsCollisionSize.X;
    }
    if (ShipSpatial->Position.X > LimitMax.X - Ship->BoundsCollisionSize.X)
    {
        ShipSpatial->Position.X = LimitMax.X - Ship->BoundsCollisionSize.X;
    }
    if (ShipSpatial->Position.Y < LimitMin.Y + Ship->BoundsCollisionSize.Y)
    {
        ShipSpatial->Position.Y = LimitMin.Y + Ship->BoundsCollisionSize.Y;
    }
    if (ShipSpatial->Position.Y > LimitMax.Y - Ship->BoundsCollisionSize.Y)
    {
        ShipSpatial->Position.Y = LimitMax.Y - Ship->BoundsCollisionSize.Y;
    }

    if (Input->X.Pressed)
    {
        UseBomb(World);
    }

    weapon* Weapon_Normal = &World->Weapons[Ship->WeaponId_Normal];
    weapon* Weapon_Focus = &World->Weapons[Ship->WeaponId_Focus];
    UpdateWeapon(Weapon_Normal, ShipSpatial->Velocity, Input->DeltaTime);
    UpdateWeapon(Weapon_Focus, ShipSpatial->Velocity, Input->DeltaTime);

    weapon* CurrentWeapon = Ship->ControlMode == ShipControlMode_Focus ?
        Weapon_Focus :
        Weapon_Normal;

    CurrentWeapon->FireCooldownRemaining -= Input->DeltaTime;
    if (CurrentWeapon->FireCooldownRemaining <= 0.0f)
    {
        if (!Ship->Firing && Input->Z.Pressed)
        {
            Ship->Firing = true;
        }
        else if (!Input->Z.Down)
        {
            Ship->Firing = false;
        }
        if (Ship->Firing && Input->Z.Down)
        {
            for (uint32 ShotIndex = 0;
                 ShotIndex < CurrentWeapon->ShotCount;
                 ++ShotIndex)
            {
                real32 PerturbedAngle = RandomPerturb(GameState, -90.0f, CurrentWeapon->Inaccuracy);
                v2 ShotPosition =
                    ShipSpatial->Position +
                    CurrentWeapon->ShotOffsets[ShotIndex] +
                    CurrentWeapon->Offset_Delay +
                    CurrentWeapon->Offset_Kickback;

                CreateBullet_Angle(GameState,
                                   ShotPosition,
                                   PerturbedAngle,
                                   CurrentWeapon->FireSpeed,
                                   0.0f, 0.0f,
                                   0,
                                   CurrentWeapon->ProjectileSpriteId,
                                   Colors.Yellow,
                                   PlayerLayer + 2,
                                   0, 2,
                                   Target_AI);
                // TODO: A muzzle flash that isn't shit...
                CreateParticle(World,
                               ShotPosition + V2(0.0f, -6.0f),
                               V2(0.0f, 0.0f),
                               1, 1.0f,
                               ColorSpec_None,
                               PlayerLayer + 1, TextureId_MuzzleFlash);
            }
            CurrentWeapon->FireCooldownRemaining = CurrentWeapon->FireCooldown;
            CurrentWeapon->Offset_Kickback.Y = CurrentWeapon->FireKickback;
            //ScreenShake(GameState->Renderer, 1.0f, 0.05f);
        }
    }

    UpdateSpriteArray(GameState, Ship->Sprites, Ship->SpriteCount);
    UpdateSpriteArray(GameState, Ship->FocusSprites, Ship->FocusSpriteCount);

    // Render
    if (Ship->Dead)
    {
    }
    else
    {
        v2 RenderPosition = ShipSpatial->Position + Ship->RenderOffset;
        RenderWeapon(GameState, Weapon_Normal, RenderPosition, PlayerLayer);
        RenderWeapon(GameState, Weapon_Focus, RenderPosition, PlayerLayer);

        if (Ship->ControlMode == ShipControlMode_Focus)
        {
            RenderSpriteArray(GameState,
                              Ship->FocusSprites, Ship->FocusSpriteCount,
                              RenderPosition,
                              PlayerLayer);
        }
        else
        {
            RenderSpriteArray(GameState,
                              Ship->Sprites, Ship->SpriteCount,
                              RenderPosition,
                              PlayerLayer);
        }
    }
}
