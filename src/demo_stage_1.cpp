#include "demo.h"
#include "stage_utils.h"

void
BulletBehavior_Cirno_Spell2_TransformedIce(game_state* GameState, projectile* Projectile, int32 Time)
{
    Event_BeginTimer(Time);

    real32 Angle = GetStoredVariable(Projectile, 0);
    Event_At(120)
    {
        SetAcceleration(Projectile, Angle, 0.025f, 1.7f);
    }
}

void
BulletBehavior_Cirno_Spell2_TransformingIce(game_state* GameState, projectile* Projectile, int32 Time)
{
    Event_BeginTimer(Time);

    // Shake
    int32 ShakeTime = (int32)GetStoredVariable(Projectile, 0);
    int32 TransformTime = (int32)GetStoredVariable(Projectile, 1);
    Event_At(ShakeTime)
    {
        Projectile_StartShake(Projectile, 4.0f);
    }

    // Transform
    Event_At(TransformTime)
    {
        PlaySound(GameState, SoundId_Freeze, 1.0f);
        projectile* TransformedIce = CreateBullet_AngleLinear(GameState,
                                                              GetPosition(Projectile),
                                                              0.0f,
                                                              0.0f,
                                                              0,
                                                              ProjectileSprite_Ball_M_Add,
                                                              Colors.Blue,
                                                              Layer_ProjectileDefault,
                                                              BulletBehavior_Cirno_Spell2_TransformedIce,
                                                              1, Target_Player);
        SetStoredVariable(TransformedIce, 0, GetAngle(Projectile));
        DestroyProjectile(GameState, Projectile);
    }
}

void
BulletBehavior_Cirno_Spell2_ConvertingStar(game_state* GameState, projectile* Projectile, int32 Time)
{
    // TODO: This is a lot of typing just to loop through projectiles. How to compress?
    uint32* ProjectileIds = CreateArray(GameState, GetActiveProjectileCount(GameState), uint32);
    uint32 ProjectileCount = 0;
    GetActiveProjectilesInCircle(GameState, GetPosition(Projectile), 15.0f, ProjectileIds, &ProjectileCount);
    for (uint32 ProjectileIndex = 0;
         ProjectileIndex < ProjectileCount;
         ++ProjectileIndex)
    {
        projectile* TestProjectile = GetProjectileById(GameState, ProjectileIds[ProjectileIndex]);
        if (GetType(TestProjectile) == Projectile_Bullet &&
            GetSpriteId(TestProjectile) == ProjectileSprite_Ball_M_Add)
        {
            PlaySound(GameState, SoundId_ShotA, 1.0f);
            CreateBullet_Angle(GameState,
                               GetPosition(TestProjectile),
                               RandomRange(GameState, 360.0f),
                               2.0f,
                               0.12f,
                               0.8f,
                               0,
                               ProjectileSprite_Ball_SS,
                               Colors.White,
                               Layer_ProjectileDefault,
                               0, 1, Target_Player);

            DestroyProjectile(GameState, TestProjectile);
        }
    }
}

void
BossBehavior_Cirno_Spell2(game_state* GameState, boss* Boss, int32 Time)
{
    int32 MoveTime = 60;
    {
        Event_BeginTimer(Time);
        Event_At(0)
        {
            v2 BossPosition = GetPosition(Boss);
            SetStoredVariable(Boss, 0, BossPosition.X);
            SetStoredVariable(Boss, 1, BossPosition.Y);
        }
        Event_FromTo(0, MoveTime, 1)
        {
            v2 OriginalPosition = V2(GetStoredVariable(Boss, 0), GetStoredVariable(Boss, 1));
            Event_MoveTo_QuadOut(Boss,
                                 OriginalPosition,
                                 V2(WorldSizeX / 2.0f, 100),
                                 Min((real32)Timer/((real32)MoveTime), 1.0f));
        }
    }

    {
        Event_BeginRepeatingTimer(Time - MoveTime, 360);

        real32 AngleToPlayer = GetAngleToPlayer(GameState, GetPosition(Boss));
        int32 EventDuration = 120;
        int32 EventStep = 5;
        int32 BulletWaveTransformDelay = 2;
        Event_FromTo(0, EventDuration, EventStep)
        {
            PlaySound(GameState, SoundId_ShotB, 1.0f);

            real32 AnglePerturbRate = 24.0f / Event_LoopIterationMax;
            real32 AnglePerturb = AnglePerturbRate * Event_LoopIteration;

            real32 Radius = 80.0f + (-2.5f * Event_LoopIteration);
            real32 Speed = 2.0f + (-0.08f * Event_LoopIteration);
            int32 Directions = 15;
            For(DirectionIndex, Directions)
            {
                real32 Angle = AngleToPlayer + (360.0f * (real32)DirectionIndex / (real32)Directions) + RandomPerturb(GameState, 0.0f, AnglePerturb);
                v2 Position = GetPosition(Boss) + V2(Radius * Cos(Angle), Radius * Sin(Angle));
                real32 BulletSpeed = Speed * RandomPerturb(GameState, 1.0f, 0.05f);
                projectile* TransformingIce = CreateBullet_AngleLinear(GameState,
                                                                       Position,
                                                                       Angle + RandomPerturb(GameState, 0.0f, 5.0f),
                                                                       Speed,
                                                                       15,
                                                                       ProjectileSprite_Ice,
                                                                       Colors.White,
                                                                       Layer_ProjectileDefault,
                                                                       BulletBehavior_Cirno_Spell2_TransformingIce,
                                                                       1, Target_Player);
                SetStoredVariable(TransformingIce, 0, (real32)(140 - (EventStep * Event_LoopIteration)));
                SetStoredVariable(TransformingIce, 1, (real32)(200 - (EventStep * Event_LoopIteration) + (BulletWaveTransformDelay * (Event_LoopIterationMax - Event_LoopIteration))));
            }
        }

        Event_At(120)
        {
            PlaySound(GameState, SoundId_ChargeC, 1.0f);
        }

        Event_FromTo(270, 300, 3)
        {
            PlaySound(GameState, SoundId_LaserA, 1.0f);
            real32 AngleIncrement = 360.0f / Event_LoopIterationMax;
            real32 Angle = AngleToPlayer + (AngleIncrement * Event_LoopIteration) - 180.0f;
            real32 Speed = 5.0f;
            real32 LaserLength = 200.0f;
            real32 LaserWidth = 13.0f;
            CreateLooseLaser(GameState,
                             GetPosition(Boss),
                             Angle,
                             Speed,
                             LaserWidth,
                             LaserLength,
                             0,
                             ProjectileSprite_Ball_M,
                             Colors.White,
                             Layer_ProjectileDefault, 0);
            CreateBullet_AngleLinear(GameState,
                                     GetPosition(Boss),
                                     Angle,
                                     Speed,
                                     0,
                                     ProjectileSprite_Star_M,
                                     Colors.White,
                                     Layer_ProjectileDefault - 1,
                                     BulletBehavior_Cirno_Spell2_ConvertingStar,
                                     1, Target_Player);
        }
    }
}

void
LaserBehavior_Kogasa_Spell3(game_state* GameState, projectile* Projectile, int32 Time)
{
    Event_BeginTimer(Time);

    real32 OriginalAngle = GetStoredVariable(Projectile, 0);
    Event_FromTo(0, 120, 1)
    {
        int32 Period = 3;
        SetStoredVariable(Projectile, 1, (real32)((((int32)GetStoredVariable(Projectile, 1) + 180 + Period) % 360) - 180.0f));
        SetStoredVariable(Projectile, 2, 15.0f * Event_LoopIterationPercent * Sin(GetStoredVariable(Projectile, 1)));
        StraightLaser_SetAngle(Projectile, OriginalAngle + GetStoredVariable(Projectile, 2));
    }

    Event_At(120)
    {
        PlaySound(GameState, SoundId_ShotD, 1.0f);
    }

    Event_FromTo(120, 180, 1)
    {
        real32 Angle = GetStoredVariable(Projectile, 1) - GetStoredVariable(Projectile, 2);
        StraightLaser_SetAngle(Projectile, OriginalAngle + GetStoredVariable(Projectile, 2) + (Angle * Sin(90.0f * (Event_LoopIteration + 1) / 60.0f)));
    }

    int32 FlowerCount = 3;
    int32 FlowerDelay = 5;
    Event_Loop(180, FlowerCount, FlowerDelay)
    {
        PlaySound(GameState, SoundId_ShotB, 1.0f);
        int32 PetalCount = 6;
        real32 AngleBase = RandomRange(GameState, 360.0f);
        For (PetalIndex, PetalCount)
        {
            real32 Radius = 90.0f * (Event_LoopIteration + 1);
            v2 Position = GetPosition(Projectile);
            v2 SpawnPosition = V2(Position.X + (Radius * Cos(StraightLaser_GetAngle(Projectile))),
                                  Position.Y + (Radius * Sin(StraightLaser_GetAngle(Projectile))));
            real32 Angle = AngleBase + (360 * PetalIndex / PetalCount);
            real32 Speed = 0.3f;
            real32 Acceleration = 1.5f / 360.0f;
            real32 SpeedMax = 1.8f;
            int32 Delay = 10;
            CreateBulletArc(GameState,
                            3, 3.5f,
                            SpawnPosition,
                            0.0f,
                            Angle,
                            Speed,
                            Acceleration,
                            SpeedMax,
                            Delay,
                            ProjectileSprite_Rice_S,
                            Colors.Cyan,
                            1, Target_Player);
        }
    }
}

void
BossBehavior_Kogasa_Spell3(game_state* GameState, boss* Boss, int32 Time)
{
    SetBulletBounds(GameState, 300.0f);

    int32 MoveTime = 120;
    {
        Event_BeginTimer(Time);
        Event_At(0)
        {
            v2 BossPosition = GetPosition(Boss);
            SetStoredVariable(Boss, 0, BossPosition.X);
            SetStoredVariable(Boss, 1, BossPosition.Y);
        }
        v2 InitialBossPosition = V2(GetStoredVariable(Boss, 0),
                                    GetStoredVariable(Boss, 1));
        Event_MoveTo_QuadOut(Boss,
                             InitialBossPosition,
                             V2(WorldSizeX / 2.0f, WorldSizeY / 2.0f),
                             Min((real32)Timer/((real32)MoveTime * 1.0f), 1.0f));
    }

    {
        Event_BeginRepeatingTimer(Time - MoveTime, 420);

        real32 AngleToPlayer = GetAngleToPlayer(GameState, GetPosition(Boss));
        int32 LaserDelay = 60;
        Event_At(0)
        {
            PlaySound(GameState, SoundId_ChargeC, 1.0f);
            int32 LaserCount = 8;
            For(LaserIndex, LaserCount)
            {
                // Target angle
                real32 Length = 480.0f;
                real32 Width = 15.0f;
                int32 TimeToLive = 240;
                projectile* Projectile = CreateStraightLaser(GameState,
                                                             GetPosition(Boss),
                                                             AngleToPlayer,
                                                             Length, Width, TimeToLive, LaserDelay,
                                                             ProjectileSprite_Laser,
                                                             Colors.Purple,
                                                             Layer_LaserDefault,
                                                             LaserBehavior_Kogasa_Spell3);
                Projectile_SetAlpha(Projectile, 0.8f);
                SetStoredVariable(Projectile, 0, AngleToPlayer);
                SetStoredVariable(Projectile, 1, -180.0f + 360.0f * ((real32)LaserIndex + 0.5f) / (real32)LaserCount);
            }
        }

        Event_At(LaserDelay)
        {
            PlaySound(GameState, SoundId_LaserA, 1.0f);
        }
    }
}

void
BulletBehavior_Meirin_Spell1_Spiral(game_state* GameState, projectile* Projectile, int32 Time)
{
    Event_BeginTimer(Time);

    real32 NewSpeed = GetStoredVariable(Projectile, 0);
    real32 NewAngle = GetStoredVariable(Projectile, 1);
    Event_At(60)
    {
        SetVelocity(Projectile, NewSpeed * GetDirectionFromAngle(NewAngle));
        SetAcceleration(Projectile, 0.0f, 0.0f, 0.0f);
    }
}

void
Meirin_Spell1_Spiral(game_state* GameState,
                     v2 Position,
                     int32 WaveIndex,
                     int32 BulletPerWave,
                     real32 Speed,
                     real32 Speed2,
                     real32 Angle,
                     real32 AngularAcceleration,
                     real32 AngleP)
{
    PlaySound(GameState, SoundId_ShotA, 0.7f);
    color BulletColor = ColorList[WaveIndex % 7];
    For(BulletIndex, BulletPerWave)
    {
        real32 A = Angle + (AngularAcceleration * WaveIndex) + (360 * BulletIndex / BulletPerWave);
        projectile* Projectile = CreateBullet_Angle(GameState,
                                                    Position,
                                                    A, Speed,
                                                    -Speed / 60.0f,
                                                    0.0f,
                                                    0,
                                                    ProjectileSprite_Ice,
                                                    BulletColor,
                                                    Layer_ProjectileDefault,
                                                    BulletBehavior_Meirin_Spell1_Spiral,
                                                    1, Target_Player);
        SetStoredVariable(Projectile, 0, Speed2);
        SetStoredVariable(Projectile, 1, A + AngleP);
    }
}

void
Meirin_Spell1_Curtain(game_state* GameState,
                      v2 BossPosition,
                      real32 I,
                      real32 J,
                      real32 DirectionModifier,
                      int32 BulletsPerWave,
                      real32 OffsetRadius,
                      real32 Arc,
                      real32 Speed,
                      real32 AngularVelocity,
                      real32 AccelerationY,
                      real32 SpeedMaxY)
{
    PlaySound(GameState, SoundId_ShotA, 0.7f);
    For(BulletIndex, BulletsPerWave)
    {
        real32 SpawnAngle = 270.0f + (Arc * J) * DirectionModifier;
        real32 VelocityAngle = SpawnAngle - (AngularVelocity * I) * DirectionModifier;
        v2 PositionOffset = V2(OffsetRadius * Cos(SpawnAngle), OffsetRadius * Sin(SpawnAngle));
        v2 Velocity = V2(Speed * Cos(VelocityAngle), Speed * Sin(VelocityAngle));
        v2 Acceleration = V2(0.0f, AccelerationY);
        v2 SpeedMax = V2(BulletSpeedMax, SpeedMaxY);
        color BulletColor = ColorList[BulletIndex % 7];
        CreateBullet(GameState,
                     BossPosition + PositionOffset,
                     Velocity,
                     Acceleration,
                     SpeedMax,
                     0,
                     ProjectileSprite_Rice_S,
                     BulletColor,
                     Layer_ProjectileDefault,
                     0, 1, Target_Player);
        ++I;
    }
}

void
BossBehavior_Meirin_Spell1(game_state* GameState, boss* Boss, int32 Time)
{
    int32 MoveTime = 120;
    {
        Event_BeginTimer(Time);
        Event_At(0)
        {
            SetStoredVariable(Boss, 2, 1.0f);

            v2 BossPosition = GetPosition(Boss);
            SetStoredVariable(Boss, 0, BossPosition.X);
            SetStoredVariable(Boss, 1, BossPosition.Y);
        }
        v2 InitialBossPosition = V2(GetStoredVariable(Boss, 0),
                                    GetStoredVariable(Boss, 1));
        Event_FromTo(0, MoveTime, 1)
        {
            Event_MoveTo_QuadOut(Boss,
                                 InitialBossPosition,
                                 V2(WorldSizeX / 2.0f, 100.0f),
                                 Min((real32)Timer/((real32)MoveTime * 1.0f), 1.0f));
        }
    }

    {
        Event_BeginRepeatingTimer(Time - MoveTime, 360);

        real32 DirectionModifier = GetStoredVariable(Boss, 2);
        v2 BossPosition = GetPosition(Boss);
        real32 AngleToPlayer = GetAngleToPlayer(GameState, GetPosition(Boss));

        real32 BaseAcceleration = 7.7f * DirectionModifier;
        int32 BulletsPerWave = 5;
        real32 DelayedAngleChange = 120.0f * DirectionModifier;
        real32 AlternateAngle = AngleToPlayer + (180.0f / BulletsPerWave * DirectionModifier);

        Event_At(0)
        {
            PlaySound(GameState, SoundId_ShotD, 1.0f);
        }

        Event_Loop(0, 36, 5)
        {
            real32 Speed = 5.0f;
            real32 Acceleration = 2.0f;
            Meirin_Spell1_Spiral(GameState, BossPosition, Event_LoopIteration, BulletsPerWave,
                                 Speed, Acceleration,
                                 AngleToPlayer,
                                 -BaseAcceleration,
                                 DelayedAngleChange);

            Meirin_Spell1_Spiral(GameState, BossPosition, Event_LoopIteration, BulletsPerWave,
                                 Speed, Acceleration,
                                 AlternateAngle,
                                 BaseAcceleration,
                                 -DelayedAngleChange);
        }
        Event_Loop(0, 9, 20)
        {
            real32 Speed = 3.0f;
            real32 Acceleration = 1.0f;
            Meirin_Spell1_Spiral(GameState, BossPosition, Event_LoopIteration, BulletsPerWave,
                                 Speed, Acceleration,
                                 AngleToPlayer,
                                 -BaseAcceleration * 2.0f,
                                 DelayedAngleChange);

            Meirin_Spell1_Spiral(GameState, BossPosition, Event_LoopIteration, BulletsPerWave,
                                 Speed, Acceleration,
                                 AlternateAngle,
                                 BaseAcceleration * 2.0f,
                                 -DelayedAngleChange);
        }

        Event_Loop(240, 20, 3)
        {
            BulletsPerWave = 14;
            real32 OffsetRadius = 30.0f;
            real32 Arc = 5.0f;
            real32 I = -1.0f * (BulletsPerWave - 1) / 2.0f;
            real32 J = (-1.0f * (Event_LoopIterationMax - 1) / 2.0f) + Event_LoopIteration;
            real32 Speed = 3.0f;
            real32 AngularVelocity = 10.0f;
            real32 AccelerationY = 0.05f;
            real32 SpeedMaxY = 10.0f;
            Meirin_Spell1_Curtain(GameState,
                                  BossPosition,
                                  I, J, DirectionModifier,
                                  BulletsPerWave, OffsetRadius, Arc,
                                  Speed, AngularVelocity,
                                  AccelerationY,
                                  SpeedMaxY);
        }

        int32 MoveTime = 60;
        Event_At(300)
        {
            // NOTE: Change direction
            SetStoredVariable(Boss, 2, -GetStoredVariable(Boss, 2));

            BossPosition = GetPosition(Boss);
            v2 TentativeMoveDestination = V2(GetPlayerPosition(GameState).X, BossPosition.Y);
            v2 MoveTrajectory = TentativeMoveDestination - BossPosition;
            v2 CappedMoveTrajectory = ClampMagnitude(MoveTrajectory, 50.0f);
            v2 CappedMoveDestination = BossPosition + CappedMoveTrajectory;
            rect MovementBounds = Rect(100.0f, 100.0f, WorldSizeX - 100.0f, WorldSizeY - 100.0f);
            v2 BoundedMoveDestination = ClampBounds(CappedMoveDestination, MovementBounds);
            SetStoredVariable(Boss, 3, BossPosition.X);
            SetStoredVariable(Boss, 4, BossPosition.Y);
            SetStoredVariable(Boss, 5, BoundedMoveDestination.X);
            SetStoredVariable(Boss, 6, BoundedMoveDestination.Y);
        }
        Event_FromTo(300, 300 + MoveTime, 1)
        {
            v2 OriginalBossPosition = V2(GetStoredVariable(Boss, 3), GetStoredVariable(Boss, 4));
            v2 MoveDestination = V2(GetStoredVariable(Boss, 5), GetStoredVariable(Boss, 6));
            Event_MoveTo_QuadOut(Boss,
                                 OriginalBossPosition,
                                 MoveDestination,
                                 Min((real32)(Timer - 300)/((real32)MoveTime), 1.0f));
        }
    }
}

void
BossBehavior_Meirin_Spell3(game_state* GameState, boss* Boss, int32 Time)
{
    Event_BeginRepeatingTimer(Time, 360);

    Event_At(0)
    {
        PlaySound(GameState, SoundId_ChargeC, 1.0f);
        real32 AngleToPlayer = GetAngleToPlayer(GameState, GetPosition(Boss));
        SetStoredVariable(Boss, 0, AngleToPlayer);
        real32 DirectionModifier = 1.0f;
        SetStoredVariable(Boss, 1, DirectionModifier);
    }

    real32 AngleToPlayer = GetStoredVariable(Boss, 0);
    real32 DirectionModifier = GetStoredVariable(Boss, 1);

    Event_FromTo(60, 110, 10)
    {
        PlaySound(GameState, SoundId_ShotA, 1.0f);
        PlaySound(GameState, SoundId_ShotD, 1.0f);
        int32 SquareShotCount = 7;
        real32 rRate = 0.5f;
        v2 VelocityOffset = V2(300.0f - (45.0f * Event_LoopIteration), 0.0f);
        real32 BaseSpeed = (45.0f - (3.0f * Event_LoopIteration)) * rRate;
        real32 BulletsPerSide = 8.0f + Event_LoopIteration;
        real32 BaseAngle = AngleToPlayer + (DirectionModifier * (360.0f * Event_LoopIteration / 14.0f));
        real32 VelocityFactor = 1.0f / 60.0f;
        int32 Delay = 10;
        For(SquareShotIndex, SquareShotCount)
        {
            real32 SquareAngle = BaseAngle + (360.0f * (real32)SquareShotIndex / (real32)SquareShotCount);
            color SquareColor = ColorList[SquareShotIndex % 7];
            For(SquareSideIndex, 4)
            {
                v2 BaseVelocity = V2(BaseSpeed * Cos(90.0f * SquareSideIndex),
                                     BaseSpeed * Sin(90.0f * SquareSideIndex));
                v2 AdditionalVelocity = V2(BaseSpeed * Cos(90.0f * (SquareSideIndex + 1)),
                                           BaseSpeed * Sin(90.0f * (SquareSideIndex + 1))) - BaseVelocity;
                For(BulletIndex, BulletsPerSide)
                {
                    real32 SidePercentage = (real32)BulletIndex / (real32)BulletsPerSide;
                    v2 Velocity = BaseVelocity + (SidePercentage * AdditionalVelocity) + VelocityOffset;
                    real32 Angle = VectorAngle(Velocity);
                    real32 Speed = Magnitude(Velocity) * VelocityFactor;
                    CreateBullet_AngleLinear(GameState,
                                             GetPosition(Boss),
                                             SquareAngle + Angle,
                                             Speed,
                                             Delay,
                                             ProjectileSprite_Ice,
                                             SquareColor,
                                             Layer_ProjectileDefault,
                                             0, 1, Target_Player);
                }
            }
        }
    }

    Event_FromTo(180, 240, 1)
    {
        PlaySound(GameState, SoundId_ShotD, 1.0f);
        real32 BaseSpeed = 0.6f;
        int32 BulletCount = 7;
        real32 AngleRate = 15.5f * DirectionModifier;
        int32 Delay = 10;
        real32 Speed = BaseSpeed + (2.1f / 60.0f * Event_LoopIteration);
        color WaveColor = ColorList[Event_LoopIteration % 7];
        For(BulletIndex, BulletCount)
        {
            CreateBullet_Angle(GameState,
                               GetPosition(Boss),
                               AngleToPlayer + (360.0f * BulletIndex / BulletCount) + (AngleRate * Event_LoopIteration),
                               2.0f * Speed,
                               -Speed / 60.0f,
                               Speed,
                               Delay,
                               ProjectileSprite_Rice_S,
                               WaveColor,
                               Layer_ProjectileDefault,
                               0, 1, Target_Player);
        }
    }

    Event_At(300)
    {
        // Move and reverse direction
        SetStoredVariable(Boss, 1, -GetStoredVariable(Boss, 1));
    }
}

void
BossBehavior_LaserTest(game_state* GameState, boss* Boss, int32 Time)
{
    //Event_BeginRepeatingTimer(Time, 120);
    Event_BeginTimer(Time);

    Event_At(0)
    {
        real32 LaserLength = 800.0f;
        real32 LaserWidth = 40.0f;
        //real32 Speed = 5.0f * 60.0f;
        //CreateLooseLaser(GameState,
                         //Boss->Spatial.Position,
                         //110.0f,
                         //Speed,
                         //LaserWidth,
                         //LaserLength,
                         //ProjectileSprite_Ball_M);
        CreateStraightLaser(GameState,
                            GetPosition(Boss),
                            110.0f,
                            LaserLength, LaserWidth,
                            3 * 60,
                            60,
                            ProjectileSprite_Laser, Colors.Red, Layer_LaserDefault, 0);
    }
}

void
UpdateStage1(game_state* GameState, int32 Time)
{
    Event_BeginTimer(Time);

    Event_At(0)
    {
        boss* Boss = CreateBoss(GameState, 32.0f, 20.0f);

        AddBossAttack(Boss, BossBehavior_Cirno_Spell2, 400, TimeToFrames(90.0f));
        AddBossAttack(Boss, BossBehavior_Kogasa_Spell3, 300, TimeToFrames(90.0f));
        AddBossAttack(Boss, BossBehavior_Meirin_Spell1, 400, TimeToFrames(90.0f));
        //AddBossAttack(Boss, BossBehavior_Meirin_Spell3, 200, TimeToFrames(90.0f));
    }

    boss* Boss = GetBoss(GameState);
    if (Boss && !IsActive(Boss))
    {
        // TODO: Need boss's time of death to lerp the alpha
        real32 TextAlpha = 1.0f;
        RenderText(GameState, "You won...", V2(WorldSizeX / 2.0f, 100.0f), Colors.White, true, 20);
        RenderText(GameState, "But you were the villain all along.", V2(WorldSizeX / 2.0f, 120.0f), Colors.White, true, 20);
    }
}
