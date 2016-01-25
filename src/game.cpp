// TODO: Won't need this here when we properly separate user code
#include "demo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "game.h"
#include "world.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "render.h"
#include "entity.h"
#include "menu.h"
#include <cstring>

#include "texture.h"
#include "shader.h"

enum main_menu_screen
{
    MainMenuScreen_MainMenu,
    MainMenuScreen_Controls,
};

struct game_mode_main_menu
{
    main_menu_screen Screen;

    menu_grid MainMenu;
    menu_grid ControlsMenu;
};

// TODO: Move menu functions
bool32 MenuFunction_MainMenu_StartGame(game_state* GameState);
bool32 MenuFunction_MainMenu_Controls(game_state* GameState);
bool32 MenuFunction_MainMenu_Exit(game_state* GameState);
bool32 MenuFunction_Controls_Back(game_state* GameState);
bool32 MenuFunction_Dead_Restart(game_state* GameState);
bool32 MenuFunction_Dead_MainMenu(game_state* GameState);

void
InitializeWorld(game_state* GameState)
{
    game_mode_world* World = GameState->World;
    World->ControlMode = ControlMode_Normal;

    World->Paused = false;
    World->FreezeBossTime = false;

    World->Bomb->Active = false;
    World->Bomb->Timer = 0;

    World->Timer = 0;
    World->BulletBounds = BulletBoundsDefault;

    InitializePlayer(World);
    ClearProjectiles(GameState);
    ClearParticles(World);

    World->Boss->Active = false;
}

void
BeginGameMode_MainMenu(game_state* GameState)
{
    SetGameMode(GameState, GameMode_MainMenu);

    ClearScreenShake(GameState);

    game_mode_main_menu* MainMenuMode = PushStruct(&GameState->ModeMemory, game_mode_main_menu);

    MainMenuMode->Screen = MainMenuScreen_MainMenu;

    InitializeMenu(&MainMenuMode->MainMenu, 1, true, V2(WorldSizeX, WorldSizeY) * 0.5f, EngineLayer_MainMenu);
    AddMenuEntry(&MainMenuMode->MainMenu, "Start", "+Start+", MenuFunction_MainMenu_StartGame);
    AddMenuEntry(&MainMenuMode->MainMenu, "Controls", "+Controls+", MenuFunction_MainMenu_Controls);
    AddMenuEntry(&MainMenuMode->MainMenu, "Exit", "+Exit+", MenuFunction_MainMenu_Exit);

    InitializeMenu(&MainMenuMode->ControlsMenu, 1, true, V2(0.0f, 180.0f) + V2(WorldSizeX, WorldSizeY) * 0.5f, EngineLayer_MainMenu);
    AddMenuEntry(&MainMenuMode->ControlsMenu, "Back", "+Back+", MenuFunction_Controls_Back);

    GameState->MainMenu = MainMenuMode;
}

void
BeginGameMode_World(game_state* GameState)
{
    SetGameMode(GameState, GameMode_World);

    ClearScreenShake(GameState);

    game_mode_world* World = PushStruct(&GameState->ModeMemory, game_mode_world);
    DivideMemoryZone(&World->Memory, &GameState->ModeMemory, GetMemoryZoneRemaining(&GameState->ModeMemory));

    World->DebugOverlay = false;

    // TODO: Possibly move to user side
    World->RestartMenu = PushStruct(&World->Memory, menu_grid);
    InitializeMenu(World->RestartMenu, 1, true, V2(WorldSizeX, WorldSizeY) * 0.5f, EngineLayer_RestartMenu);
    AddMenuEntry(World->RestartMenu, "Restart", "+Restart+", MenuFunction_Dead_Restart);
    AddMenuEntry(World->RestartMenu, "Main Menu", "+Main Menu+", MenuFunction_Dead_MainMenu);

    World->Player = PushStruct(&World->Memory, player);
    World->Ships = PushArray(&World->Memory, ShipId_Count, ship);
    World->Weapons = PushArray(&World->Memory, WeaponId_Count, weapon);
    World->Bomb = PushStruct(&World->Memory, bomb);
    World->Projectiles = PushArray(&World->Memory, ProjectileCountMax, projectile);
    World->Particles = PushArray(&World->Memory, ParticleCountMax, particle);
    World->Enemies = PushArray(&World->Memory, EnemyCountMax, enemy);
    World->Boss = PushStruct(&World->Memory, boss);
    World->ScrollingBackground = PushStruct(&World->Memory, scrolling_background);

    DefineGame(GameState);

    InitializeWorld(GameState);
    GameState->World = World;
}

bool32
MenuFunction_MainMenu_StartGame(game_state* GameState)
{
    BeginGameMode_World(GameState);
    return true;
}

bool32
MenuFunction_MainMenu_Controls(game_state* GameState)
{
    GameState->MainMenu->Screen = MainMenuScreen_Controls;
    return false;
}

bool32
MenuFunction_MainMenu_Exit(game_state* GameState)
{
    GameState->Running = false;
    return false;
}

bool32
MenuFunction_Controls_Back(game_state* GameState)
{
    GameState->MainMenu->Screen = MainMenuScreen_MainMenu;
    return false;
}

bool32
MenuFunction_Dead_Restart(game_state* GameState)
{
    InitializeWorld(GameState);
    return false;
}

bool32
MenuFunction_Dead_MainMenu(game_state* GameState)
{
    BeginGameMode_MainMenu(GameState);
    return true;
}

void
CheckCollisions(game_state* GameState)
{
    game_mode_world* World = GameState->World;
    ship* Ship = &World->Ships[World->Player->ShipId];
    spatial* ShipSpatial = &Ship->Spatial;
    boss* Boss = World->Boss;

    if (!Ship->Dead &&
        Ship->DamageCollisionArea.Collidable)
    {
        if (Boss->Active &&
            Boss->PlayerKillCollisionArea.Collidable)
        {
            if (Collides(ShipSpatial, &Ship->DamageCollisionArea,
                         &Boss->Spatial, &Boss->PlayerKillCollisionArea))
            {
                KillShip(GameState);
                return;
            }
        }
    }

    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCountMax;
         ++ProjectileIndex)
    {
        projectile* Projectile = &World->Projectiles[ProjectileIndex];
        if (Projectile->Active)
        {
            v2 ProjectilePosition = Projectile->Spatial.Position;
            switch(Projectile->Type)
            {
                case Projectile_Bullet:
                {
                    bullet* Bullet = &Projectile->Bullet;
                    if (Bullet->CollisionArea.Collidable)
                    {
                        if (Projectile->Target == Target_Player)
                        {
                            if (World->Bomb->Active)
                            {
                                real32 BombRadius = GetBombRadius(World);
                                if (Distance(ProjectilePosition, World->Bomb->Origin) < BombRadius + Bullet->CollisionArea.Radius)
                                {
                                    DestroyProjectile(GameState, Projectile);
                                    break;
                                }
                            }
                            if (!Ship->Dead &&
                                Ship->DamageCollisionArea.Collidable)
                            {
                                collision_area_circle* ShipCollision = &Ship->DamageCollisionArea;
                                if (Collides(ShipSpatial, ShipCollision, &Projectile->Spatial, &Bullet->CollisionArea))
                                {
                                    World->ControlMode = ControlMode_FatalCollision;
                                    World->FatalCollisionTime = Ship->FatalCollisionTime;
                                    continue;
                                }

                                if (!Projectile->Grazed &&
                                    Collides(ShipSpatial, &Ship->GrazeCollisionArea,
                                             &Projectile->Spatial, &Bullet->CollisionArea))
                                {
                                    v2 BulletToPlayer = ShipSpatial->Position - ProjectilePosition;
                                    v2 Scaled = Rescale(BulletToPlayer, Bullet->CollisionArea.Radius);
                                    CreateGrazeParticle(GameState,
                                                        ProjectilePosition + Scaled);
                                    PlaySound(GameState, SoundId_Graze, 1.0f);
                                    Projectile->Grazed = true;

                                    IncrementBombMeter(&Ship->BombModule);
                                }
                            }
                        }
                        else if (Projectile->Target == Target_AI)
                        {
                            boss* Boss = World->Boss;
                            if (Boss->Active &&
                                Collides(&Boss->Spatial, &Boss->DamageCollisionArea, &Projectile->Spatial, &Bullet->CollisionArea))
                            {
                                HitBoss(GameState, Projectile->Damage);
                                DestroyProjectile(GameState, Projectile);
                            }
                        }
                    }
                } break;
                case Projectile_LooseLaser:
                {
                    loose_laser* LooseLaser = &Projectile->LooseLaser;
                    if (World->Bomb->Active)
                    {
                        break;
                    }
                    if (!Ship->Dead &&
                        Ship->DamageCollisionArea.Collidable)
                    {
                        if (CollidesRect(&Ship->Spatial,
                                         &Ship->DamageCollisionArea,
                                         ProjectilePosition,
                                         V2(LooseLaser->Width, LooseLaser->Length),
                                         LooseLaser->Angle,
                                         LooseLaserCollisionFactor))
                        {
                            World->ControlMode = ControlMode_FatalCollision;
                            World->FatalCollisionTime = Ship->FatalCollisionTime;
                        }

                        if (!Projectile->Grazed &&
                            CollidesRect(ShipSpatial,
                                         &Ship->GrazeCollisionArea,
                                         ProjectilePosition,
                                         V2(LooseLaser->Width, LooseLaser->Length),
                                         LooseLaser->Angle,
                                         StraightLaserCollisionFactor))
                        {
                            v2 LaserDirection = GetDirectionFromAngle(LooseLaser->Angle);
                            v2 ShipToLaser = ProjectilePosition - ShipSpatial->Position;
                            v2 GrazeDirection = ShipToLaser - (Dot(ShipToLaser, LaserDirection) * LaserDirection);
                            v2 GrazePosition = Rescale(GrazeDirection, Ship->GrazeCollisionArea.Radius);
                            CreateGrazeParticle(GameState,
                                                ShipSpatial->Position + GrazePosition);
                            PlaySound(GameState, SoundId_Graze, 1.0f);
                            Projectile->Grazed = true;

                            IncrementBombMeter(&Ship->BombModule);
                        }
                    }
                } break;
                case Projectile_StraightLaser:
                {
                    straight_laser* StraightLaser = &Projectile->StraightLaser;
                    if (World->Bomb->Active)
                    {
                        break;
                    }
                    if (!Ship->Dead &&
                        Ship->DamageCollisionArea.Collidable &&
                        Projectile->Collidable)
                    {
                        if (CollidesRect(ShipSpatial,
                                         &Ship->DamageCollisionArea,
                                         ProjectilePosition,
                                         V2(StraightLaser->Width, StraightLaser->Length),
                                         StraightLaser->Angle,
                                         StraightLaserCollisionFactor))
                        {
                            World->ControlMode = ControlMode_FatalCollision;
                            World->FatalCollisionTime = Ship->FatalCollisionTime;
                        }

                        if (!Projectile->Grazed &&
                            CollidesRect(ShipSpatial,
                                         &Ship->GrazeCollisionArea,
                                         ProjectilePosition,
                                         V2(StraightLaser->Width, StraightLaser->Length),
                                         StraightLaser->Angle,
                                         StraightLaserCollisionFactor))
                        {
                            v2 LaserDirection = GetDirectionFromAngle(StraightLaser->Angle);
                            v2 ShipToLaser = ProjectilePosition - ShipSpatial->Position;
                            v2 GrazeDirection = ShipToLaser - (Dot(ShipToLaser, LaserDirection) * LaserDirection);
                            v2 GrazePosition = Rescale(GrazeDirection, Ship->GrazeCollisionArea.Radius);
                            CreateGrazeParticle(GameState,
                                                ShipSpatial->Position + GrazePosition);
                            PlaySound(GameState, SoundId_Graze, 1.0f);
                            Projectile->Grazed = true;

                            IncrementBombMeter(&Ship->BombModule);
                        }
                    }
                } break;
            }
        }
    }
}

void
UpdateBomb(game_state* GameState, real32 DeltaTime)
{
    game_mode_world* World = GameState->World;
    bomb* Bomb = World->Bomb;
    if (Bomb->Active)
    {
        ++Bomb->Timer;
        if (Bomb->Timer > Bomb->Duration)
        {
            Bomb->Active = false;
        }
        real32 Alpha = (real32)(Bomb->Duration - Bomb->Timer) / (real32)Bomb->Duration;
        PushCircle(GameState->Renderer,
                   Bomb->Origin,
                   GetBombRadius(World),
                   Color(Colors.White, Alpha),
                   false, EngineLayer_Bomb);
    }
}

void
RenderDebugCounter(renderer* Renderer,
                   v2 Position,
                   const char* FormatString,
                   int32 Count,
                   int32 CountMax)
{
    real32 DebugMeterWidth = 100.0f;
    real32 DebugMeterHeight = 0.1f;

    char Text[32];
    sprintf(Text, FormatString, Count, CountMax);
    PushText(Renderer, Text, Position, Colors.White, false, EngineLayer_DebugInfo);
    real32 Percentage = ((real32)Count / (real32)CountMax);
    color BarColor = Percentage > Debug_ObjectCounterWarningPercentage ?
        Colors.Red :
        Colors.White;
    PushRect(Renderer,
             Rect(Position + V2(0.0f, 8.0f), V2(DebugMeterWidth * Percentage, 1.0f)),
             BarColor, true, EngineLayer_DebugInfo);
}

void
UpdateDebug(game_state* GameState,
            game_input* Input)
{
    renderer* Renderer = GameState->Renderer;
    game_mode_world* World = GameState->World;
    player* Player = World->Player;
    boss* Boss = World->Boss;
    ship* Ship = &World->Ships[Player->ShipId];

    if (World->DebugOverlay)
    {
        PushRect(Renderer, Rect(0.0f, 0.0f, WorldSizeX, WorldSizeY), Color(0.0f, 0.0f, 0.0f, 0.7f), true, EngineLayer_DebugDarken);
        if (!Ship->Dead)
        {
            PushCircle(Renderer, Ship->Spatial.Position, Ship->GrazeCollisionArea.Radius, Colors.Yellow, false, EngineLayer_DebugCollisionInfo);
            PushCircle(Renderer, Ship->Spatial.Position, Ship->DamageCollisionArea.Radius, Colors.Red, false, EngineLayer_DebugCollisionInfo);
        }

        if (Boss->Active)
        {
            PushCircle(Renderer, Boss->Spatial.Position, Boss->DamageCollisionArea.Radius, Colors.Yellow, false, EngineLayer_DebugCollisionInfo);
            PushCircle(Renderer, Boss->Spatial.Position, Boss->PlayerKillCollisionArea.Radius, Colors.Red, false, EngineLayer_DebugCollisionInfo);
        }

        for (uint32 ProjectileIndex = 0;
             ProjectileIndex < ProjectileCountMax;
             ++ProjectileIndex)
        {
            projectile* Projectile = &World->Projectiles[ProjectileIndex];
            if (Projectile->Active)
            {
                switch(Projectile->Type)
                {
                    case Projectile_Bullet:
                    {
                        bullet* Bullet = &Projectile->Bullet;
                        PushRect(Renderer, Rect(Projectile->Spatial.Position, V2(0.0, 0.0)), Colors.Red, true, EngineLayer_DebugCollisionInfo);
                        PushCircle(Renderer, Projectile->Spatial.Position, Bullet->CollisionArea.Radius, Colors.Red, false, EngineLayer_DebugCollisionInfo);
                    } break;
                    case Projectile_LooseLaser:
                    {
                        loose_laser* LooseLaser = &Projectile->LooseLaser;
                        v2 LaserDirection = GetDirectionFromAngle(LooseLaser->Angle);
                        real32 LaserHalfWidth = LooseLaser->Width * LooseLaserCollisionFactor.X / 2.0f;
                        v2 NormalL = V2(LaserDirection.Y, -LaserDirection.X);
                        v2 NormalR = V2(-LaserDirection.Y, LaserDirection.X);
                        real32 AdjustedLength = LooseLaser->Length * LooseLaserCollisionFactor.Y;
                        v2 AdjustedBack = Projectile->Spatial.Position + Rescale(LaserDirection, 0.5f * (LooseLaser->Length - AdjustedLength));
                        v2 LaserFront = AdjustedBack + Rescale(LaserDirection, AdjustedLength);

                        v2 VA = AdjustedBack + Rescale(NormalL, LaserHalfWidth);
                        v2 VB = LaserFront + Rescale(NormalL, LaserHalfWidth);
                        v2 VC = LaserFront + Rescale(NormalR, LaserHalfWidth);
                        v2 VD = AdjustedBack + Rescale(NormalR, LaserHalfWidth);
                        PushPolygon4(Renderer, VA, VB, VC, VD, Colors.Red, false, EngineLayer_DebugCollisionInfo);
#if 0
                        // ULTRA DEBUG: Render the pre-rotation rect and the player's relative position
                        v2 PlayerPosition = Ship->Spatial.Position;
                        v2 RotatedShipPosition = RotateAround(PlayerPosition, Laser->Back, -Laser->Angle);

                        real32 ShipRadius = Ship->Spatial.Radius;
                        v2 LaserCorner = V2(Laser->Back.X, Laser->Back.Y - Laser->Width/2.0f);
                        v2 LaserDimension = V2(Laser->Length, Laser->Width);
                        // Apply shrinking factor
                        v2 RotatedCollisionFactor = V2(LaserCollisionFactor.Y, LaserCollisionFactor.X);
                        v2 InverseCollisionFactor = V2(1.0f, 1.0f) - RotatedCollisionFactor;
                        v2 HalfInverseCollisionFactor = 0.5f * InverseCollisionFactor;
                        v2 Adjustment = Hadamard(HalfInverseCollisionFactor, LaserDimension);
                        v2 AdjustedLaserCorner = LaserCorner + Adjustment;
                        v2 AdjustedLaserDimension = LaserDimension - 2.0f * Adjustment;
                        // Add ship radius to resulting shape
                        v2 MinkowskiCorner = AdjustedLaserCorner - V2(ShipRadius, ShipRadius);
                        v2 MinkowskiDimension = AdjustedLaserDimension + V2(2 * ShipRadius, 2 * ShipRadius);
                        boxColor(GameState->Renderer->SDLRenderer,
                                 MinkowskiCorner.X, MinkowskiCorner.Y,
                                 MinkowskiCorner.X + MinkowskiDimension.X, MinkowskiCorner.Y + MinkowskiDimension.Y,
                                 0xFF0000FF);
                        boxColor(GameState->Renderer->SDLRenderer,
                                 AdjustedLaserCorner.X, AdjustedLaserCorner.Y,
                                 AdjustedLaserCorner.X + AdjustedLaserDimension.X, AdjustedLaserCorner.Y + AdjustedLaserDimension.Y,
                                 0xFF00FF00);
                        filledCircleColor(GameState->Renderer->SDLRenderer,
                                          RotatedShipPosition.X, RotatedShipPosition.Y, Ship->Spatial.Radius, 0xFF0000FF);
#endif
                    } break;
                    case Projectile_StraightLaser:
                    {
                        straight_laser* StraightLaser = &Projectile->StraightLaser;
                        if (Projectile->Collidable)
                        {
                            v2 LaserPosition = Projectile->Spatial.Position;
                            v2 LaserDirection = GetDirectionFromAngle(StraightLaser->Angle);
                            real32 LaserHalfWidth = StraightLaser->Width * StraightLaserCollisionFactor.X / 2.0f;
                            v2 NormalL = V2(LaserDirection.Y, -LaserDirection.X);
                            v2 NormalR = V2(-LaserDirection.Y, LaserDirection.X);
                            v2 LaserFront = LaserPosition + Rescale(LaserDirection, StraightLaser->Length);

                            v2 VA = LaserPosition + Rescale(NormalL, LaserHalfWidth);
                            v2 VB = LaserFront + Rescale(NormalL, LaserHalfWidth);
                            v2 VC = LaserFront + Rescale(NormalR, LaserHalfWidth);
                            v2 VD = LaserPosition + Rescale(NormalR, LaserHalfWidth);
                            PushPolygon4(Renderer, VA, VB, VC, VD, Colors.Red, false, EngineLayer_DebugCollisionInfo);
                        }
                    } break;
                }
            }
        }
    }

    PushRect(Renderer, Rect(-RenderPadding, WorldSizeY, WorldSizeX + RenderPadding, LogicalWindowSizeY - WorldSizeY + RenderPadding), Colors.Black, true, EngineLayer_DebugPanel);

    RenderDebugCounter(Renderer, V2(4, WorldSizeY + 4),  "Projectiles: %d / %d", World->ProjectileCount, ProjectileCountMax);
    RenderDebugCounter(Renderer, V2(4, WorldSizeY + 20), "Particles: %d / %d", World->ParticleCount, ParticleCountMax);
    RenderDebugCounter(Renderer, V2(4, WorldSizeY + 36), "Render Entries: %d / %d", Renderer->EntryCount, RenderEntryCountMax);

    char FPSText[16];
    sprintf(FPSText, "FPS: %.1f", Input->LastFrameFPS);
    PushText(Renderer, FPSText, V2(4, WorldSizeY + 52), Colors.White, false, EngineLayer_DebugInfo);

    real32 RightMostX = WorldSizeX / 2.0f + 20.0f;
    bomb_module* BombModule = &Ship->BombModule;
    char BombsText[32];
    sprintf(BombsText, "Bombs:");
    PushText(Renderer, BombsText, V2(RightMostX, WorldSizeY + 4), Colors.White, false, EngineLayer_DebugInfo);

    real32 LeftMostX = 0;
    for (int32 BombIndex = 0;
         BombIndex < BombModule->BombCountMax;
         ++BombIndex)
    {
        color SquareColor = BombIndex < BombModule->BombCount ?
            Colors.White :
            Color(Colors.White, 0.3f);
        rect RenderRect = Rect(RightMostX + 40.0f + (BombIndex * 10.0f), WorldSizeY + 5, 6, 6);
        PushRect(Renderer,
                 RenderRect,
                 SquareColor,
                 true, EngineLayer_DebugInfo);
        LeftMostX = RenderRect.X + RenderRect.W;
    }

    real32 MeterRenderWidth = LeftMostX - RightMostX;
    real32 MeterPercentage = (real32)BombModule->MeterValue / (real32)BombModule->MeterMax;
    PushRect(Renderer,
             Rect(RightMostX, WorldSizeY + 14.0f, MeterRenderWidth, 4),
             Color(Colors.White, 0.3f),
             true, EngineLayer_DebugInfo);
    PushRect(Renderer,
             Rect(RightMostX, WorldSizeY + 14.0f, MeterPercentage * MeterRenderWidth, 4),
             Colors.White,
             true, EngineLayer_DebugInfo);
}

void
UpdateScrollingBackground(game_state* GameState,
                          real32 DeltaTime)
{
    game_mode_world* World = GameState->World;
    scrolling_background* Background = World->ScrollingBackground;
    texture* Texture = &GameState->Assets.Textures[Background->TextureId];
    Background->SourceY -= RoundReal32(Background->ScrollSpeed * DeltaTime);
    if (Background->SourceY < Background->TextureRepeatY)
    {
        Background->SourceY = Texture->Height - WorldSizeY;
    }

    rect SourceRect = Rect(0,
                           Background->SourceY,
                           LogicalWindowSizeX + 32,
                           LogicalWindowSizeY);

    v2 DestPosition = V2(0.0f, 0.0f);

    rect DestRect = Rect(0.0f, 0.0f,
                         LogicalWindowSizeX + 32,
                         LogicalWindowSizeY);

    PushTextureEx(GameState->Renderer,
                  Background->TextureId,
                  SourceRect,
                  DestRect,
                  V2(0.0f, 0.0f),
                  V3(0.0f, 0.0f, 0.0f),
                  ColorSpec(Color(0.1f, 0.1f, 0.1f, 1.0f), ColorTint_Mod),
                  EngineLayer_Background,
                  BlendMode_Blend);
}

void
UpdateWorld(game_state* GameState, game_input* Input)
{
    real32 DeltaTime = Input->DeltaTime;
    game_mode_world* World = GameState->World;

    UpdateBomb(GameState, DeltaTime);
    UpdateScrollingBackground(GameState, DeltaTime);
    UpdatePlayer(GameState, Input);

    // User script
    temporary_memory UserMemory = BeginTemporaryMemory(&GameState->TransientState->Memory);
    GameState->World->UpdateFunction(GameState, World->Timer);
    EndTemporaryMemory(&UserMemory);

    UpdateEnemies(GameState, World->Timer);
    UpdateProjectiles(GameState, World->Timer, World->BulletBounds);
    ++World->Timer;

    CheckCollisions(GameState);
    UpdateParticles(GameState, DeltaTime);

    UpdateScreenShakes(&GameState->Renderer->PostProcessor, DeltaTime);
    UpdateDebug(GameState, Input);
}

bool32
UpdateControlMode_DeathAnimation(game_state* GameState,
                                 game_input* Input)
{
    game_mode_world* World = GameState->World;
    GameState->Renderer->EntryCount = 0;

    --World->DeathAnimationTime;
    if (World->DeathAnimationTime <= 0)
    {
        World->ControlMode = ControlMode_DeathMenu;
    }
    UpdateWorld(GameState, Input);

    return false;
}

bool32
UpdateControlMode_Normal(game_state* GameState,
                         game_input* Input)
{
    if (Input->Esc.Pressed)
    {
        BeginGameMode_MainMenu(GameState);
        return true;
    }

    GameState->Renderer->EntryCount = 0;
    UpdateWorld(GameState, Input);

    return false;
}

bool32
UpdateControlMode_FatalCollision(game_state* GameState,
                                 game_input* Input)
{
    game_mode_world* World = GameState->World;
    --World->FatalCollisionTime;
    if (World->FatalCollisionTime > 0)
    {
        if (Input->X.Pressed)
        {
            UseBomb(World);
        }
    }
    else
    {
        KillShip(GameState);
    }

    return false;
}

bool32
UpdateControlMode_DeathMenu(game_state* GameState,
                            game_input* Input)
{
    if (Input->Esc.Pressed)
    {
        BeginGameMode_MainMenu(GameState);
        return true;
    }

    GameState->Renderer->EntryCount = 0;
    game_mode_world* World = GameState->World;

    UpdateWorld(GameState, Input);

    PushRect(GameState->Renderer,
             Rect(0.0f, 0.0f, LogicalWindowSizeX, LogicalWindowSizeY),
             Color(Colors.Black, 0.4f),
             true, EngineLayer_DeathMenu);
    bool32 ModeChanged = false;
    ModeChanged = UpdateMenu(GameState,  World->RestartMenu, Input);
    return ModeChanged;
}

bool32
UpdateMode_MainMenu(game_state* GameState,
                    game_input* Input)
{
    if (Input->Esc.Pressed)
    {
        GameState->Running = false;
        return true;
    }

    renderer* Renderer = GameState->Renderer;
    Renderer->EntryCount = 0;

    bool32 ModeChanged = false;

    game_mode_main_menu* MainMenuMode = GameState->MainMenu;
    switch (MainMenuMode->Screen)
    {
        case MainMenuScreen_MainMenu:
        {
            ModeChanged = UpdateMenu(GameState, &MainMenuMode->MainMenu, Input);
        } break;
        case MainMenuScreen_Controls:
        {
            game_mode_main_menu* MainMenuMode = GameState->MainMenu;

            const char* ShipControls[] = {
                "Fire: Z",
                "Bomb: X",
                "Focus: Shift"
            };
            for (uint32 ItemIndex = 0;
                 ItemIndex < ArrayCount(ShipControls);
                 ++ItemIndex)
            {
                PushText(Renderer, ShipControls[ItemIndex],
                         V2(WorldSizeX / 2.0f, 160.0f + (ItemIndex * 20.0f)),
                         Colors.White, true, EngineLayer_MainMenu);
            }

            const char* MetaControls[] = {
                "Debug: D",
                "Pause: P",
                "Step: N",
                "Window Size: W",
                "Exit: Esc"
            };
            for (uint32 ItemIndex = 0;
                 ItemIndex < ArrayCount(MetaControls);
                 ++ItemIndex)
            {
                PushText(Renderer, MetaControls[ItemIndex],
                         V2(WorldSizeX / 2.0f, 240.0f + (ItemIndex * 20.0f)),
                         Colors.White, true, EngineLayer_MainMenu);
            }

            ModeChanged = UpdateMenu(GameState, &MainMenuMode->ControlsMenu, Input);
        } break;
    }

    return ModeChanged;
}

bool32
UpdateMode_World(game_state* GameState,
                 game_input* Input)
{
    game_mode_world* World = GameState->World;

    if (Input->Esc.Pressed)
    {
        BeginGameMode_MainMenu(GameState);
        return true;
    }

    if (Input->P.Pressed)
    {
        World->Paused = !World->Paused;
        ClearScreenShake(GameState);
    }
    if (Input->D.Pressed)
    {
        World->DebugOverlay = !World->DebugOverlay;
    }

    if (Input->K.Pressed)
    {
        if (World->Boss)
        {
            ActivateNextBossAttack(World->Boss, World->Timer);
        }
    }

    if (Input->R.Pressed)
    {
        printf("Restart.\n");
        InitializeWorld(GameState);
    }

    for (uint32 SoundIndex = 0;
         SoundIndex < SoundId_Count;
         ++SoundIndex)
    {
        sound* Sound = &GameState->Assets.Sounds[SoundIndex];
        Sound->PlayedThisFrame = false;
    }

    ship* Ship = &World->Ships[World->Player->ShipId];
    if (Input->I.Pressed)
    {
        Ship->DamageCollisionArea.Collidable = !Ship->DamageCollisionArea.Collidable;
    }

    World->FreezeBossTime = Ship->Dead;

    bool32 ModeChanged = false;

    if (!World->Paused || Input->N.Pressed)
    {
        switch(World->ControlMode)
        {
            case ControlMode_Normal:
                ModeChanged = UpdateControlMode_Normal(GameState, Input);
                break;
            case ControlMode_FatalCollision:
                ModeChanged = UpdateControlMode_FatalCollision(GameState, Input);
                break;
            case ControlMode_DeathAnimation:
                ModeChanged = UpdateControlMode_DeathAnimation(GameState, Input);
                break;
            case ControlMode_DeathMenu:
                // Note: Death menu must take the GameState because it might change mode
                ModeChanged = UpdateControlMode_DeathMenu(GameState, Input);
                break;
        }
    }

    return ModeChanged;
}

void
Update(game_state* GameState, game_input* Input)
{
    bool32 ModeChanged = false;
    switch (GameState->GameMode)
    {
        case GameMode_None:
            break;
        case GameMode_MainMenu:
            ModeChanged = UpdateMode_MainMenu(GameState, Input);
            break;
        case GameMode_World:
            ModeChanged = UpdateMode_World(GameState, Input);
            break;
    }
}

extern "C" {

int32
GameLoop(game_memory* GameMemory, game_input* Input)
{
    if (Input->ReloadedGameLibrary)
    {
        gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    }

    transient_state* TransientState = (transient_state*)GameMemory->TransientStorage;
    if (!TransientState->IsInitialized)
    {
        InitializeMemoryZone(&TransientState->Memory,
                             GameMemory->TransientStorageSize - sizeof(transient_state),
                             (uint8*)GameMemory->TransientStorage + sizeof(transient_state));

        TransientState->IsInitialized = true;
    }

    Assert("GameState too large.", sizeof(game_state) <= GameMemory->PermanentStorageSize);
    game_state* GameState = (game_state*)GameMemory->PermanentStorage;
    if (!GameState->IsInitialized)
    {
        memory_zone TotalMemory = {};
        InitializeMemoryZone(&TotalMemory,
                             GameMemory->PermanentStorageSize - sizeof(game_state),
                             (uint8*)(GameMemory->PermanentStorage) + sizeof(game_state));

        DivideMemoryZone(&GameState->AssetMemory, &TotalMemory, Megabytes(1));
        DivideMemoryZone(&GameState->RenderMemory, &TotalMemory, sizeof(renderer) + Megabytes(1));
        DivideMemoryZone(&GameState->ModeMemory, &TotalMemory, GetMemoryZoneRemaining(&TotalMemory));
        printf("Size of RenderMemory: %zu\n", GameState->RenderMemory.Size);
        printf("Size of AssetMemory: %zu\n", GameState->AssetMemory.Size);
        printf("Size of ModeMemory: %zu\n", GameState->ModeMemory.Size);

        gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

        GameState->TransientState = TransientState;

        GameState->Running = true;

        GameState->Random = PushStruct(&GameState->RenderMemory, random_mt);
        GameState->Random->MT_Index = MT_Size;
        Seed(GameState, 1234);

        GameState->GlobalVolume = 0.2f;
        GameState->Renderer = PushStruct(&GameState->RenderMemory, renderer);
        GameState->Shaders = PushArray(&GameState->RenderMemory, ShaderId_Count, shader);
        LoadShaderFromFile(GameState,
                           "src/shaders/sprite.vs",
                           0,
                           "src/shaders/sprite.fs",
                           ShaderId_Sprite);
        LoadShaderFromFile(GameState,
                           "src/shaders/text.vs",
                           0,
                           "src/shaders/text.fs",
                           ShaderId_Text);
        LoadShaderFromFile(GameState,
                           "src/shaders/rectangle.vs",
                           "src/shaders/rectangle.gs",
                           "src/shaders/geometry.fs",
                           ShaderId_Rectangle);
        LoadShaderFromFile(GameState,
                           "src/shaders/circle.vs",
                           "src/shaders/circle.gs",
                           "src/shaders/geometry.fs",
                           ShaderId_Circle);
        LoadShaderFromFile(GameState,
                           "src/shaders/polygon4.vs",
                           "src/shaders/polygon4.gs",
                           "src/shaders/geometry.fs",
                           ShaderId_Polygon4);
        LoadShaderFromFile(GameState,
                           "src/shaders/effects.vs",
                           0,
                           "src/shaders/effects.fs",
                           ShaderId_PostProcessor);

        glm::mat4 Projection = glm::ortho(0.0f,
                                          (GLfloat)(LogicalWindowSizeX),
                                          (GLfloat)(LogicalWindowSizeY),
                                          0.0f,
                                          -1000.0f, 1000.0f);
        InitializeTextureRenderer(&GameState->Renderer->TextureRenderer, &GameState->Shaders[ShaderId_Sprite], Projection);
        InitializeTextRenderer(TransientState,
                               &GameState->Renderer->TextRenderer,
                               &GameState->Shaders[ShaderId_Text],
                               Projection,
                               "data/fonts/visitor2.ttf", 13);
        InitializeRectangleRenderer(&GameState->Renderer->RectangleRenderer, &GameState->Shaders[ShaderId_Rectangle], Projection);
        InitializeCircleRenderer(&GameState->Renderer->CircleRenderer, &GameState->Shaders[ShaderId_Circle], Projection);
        InitializePolygon4Renderer(&GameState->Renderer->Polygon4Renderer, &GameState->Shaders[ShaderId_Polygon4], Projection);
        InitializePostProcessor(&GameState->Renderer->PostProcessor, &GameState->Shaders[ShaderId_PostProcessor],
                                (LogicalWindowSizeX * Input->ResizeWindow.WindowScale),
                                (LogicalWindowSizeY * Input->ResizeWindow.WindowScale),
                                Projection);

        assets* Assets = &GameState->Assets;
        Assets->Sounds = PushArray(&GameState->AssetMemory, SoundId_Count, sound);
        Assets->SoundGroups = PushArray(&GameState->AssetMemory, SoundGroupId_Count, sound_group);
        Assets->Textures = PushArray(&GameState->AssetMemory, TextureId_Count, texture);
        Assets->Animations = PushArray(&GameState->AssetMemory, AnimationSpecId_Count, animation_spec);
        Assets->BulletSprites = PushArray(&GameState->AssetMemory, ProjectileSprite_Count, bullet_sprite);

        //printf("Memory footprint: %luKB\n", WorldMemory->Used / 1000);

        GameState->IsInitialized = true;
    }

    if (Input->ResizeWindow.Active)
    {
        ResizePostProcessor(&GameState->Renderer->PostProcessor,
                            LogicalWindowSizeX * Input->ResizeWindow.WindowScale,
                            LogicalWindowSizeY * Input->ResizeWindow.WindowScale);
    }

    if (GameState->GameMode == GameMode_None)
    {
        BeginGameMode_MainMenu(GameState);
    }

    Update(GameState, Input);
    Render(GameState);

    CheckMemoryZone(&GameState->ModeMemory);
    CheckMemoryZone(&GameState->RenderMemory);
    CheckMemoryZone(&GameState->AssetMemory);
    CheckMemoryZone(&TransientState->Memory);

    return GameState->Running;
}

}
