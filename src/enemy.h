#pragma once

#include "game.h"
#include "projectile.h"
#include "player.h"
#include "particle.h"

typedef void (*enemy_behavior_function)(game_mode_world* World, enemy* Enemy, int32 Time);

struct enemy
{
    enemy_behavior_function BehaviorFunction;

    bool32 Active;
    int32 Health;

    spatial Spatial;
    collision_area_circle CollisionArea;
};

struct boss_attack
{
    int32 Health;
    int32 HealthRemaining;

    int32 StartTime;
    int32 Duration;
    int32 TimeRemaining;

    boss_behavior_function Behavior;
};

struct boss
{
    bool32 Invulnerable;

    uint32 CurrentAttackIndex;
    uint32 AttackCount;
    boss_attack Attacks[16];

    bool32 Active;

    spatial Spatial;
    collision_area_circle DamageCollisionArea;
    collision_area_circle PlayerKillCollisionArea;

    real32 StoredVariables[ProjectileStorageSize];
};

v2
Boss_GetPosition(boss* Boss)
{
    return Boss->Spatial.Position;
}

boss_attack*
GetCurrentAttack(boss* Boss)
{
    if (Boss->AttackCount == 0)
    {
        return 0;
    }
    return &Boss->Attacks[Boss->CurrentAttackIndex];
}

void
DestroyEnemy(game_mode_world* World, enemy* Enemy)
{
    Enemy->Active = false;
    --World->EnemyCount;
}

//void
//FireAtPosition(game_state* GameState, v2 Source, v2 Target, real32 Speed)
//{
    //real32 Angle = VectorAngle(Target, Source);
    //CreateBulletArc(GameState, 5, Source, 10.0f, Angle, Speed, 360.0f, BulletSprite_Ball_M_Blue, 1, Target_Player);
    //return;
//}

v2 GetVelocityForTargetSpaceTime(v2 SourcePosition, v2 TargetPosition, real32 Time)
{
    v2 MovementVector = TargetPosition - SourcePosition;
    real32 Distance = Magnitude(MovementVector);
    v2 ScaledVelocity = Rescale(MovementVector, Distance / Time);

    return ScaledVelocity;
}

void
CreateEnemy(game_state* GameState, enemy_behavior_function Behavior, int32 Health, real32 Radius)
{
    game_mode_world* World = GameState->World;
    Assert("Out of enemy memory.", World->EnemyCount < EnemyCountMax);

    bool32 Found = false;
    for (uint32 EnemyIndex = 0;
         EnemyIndex < EnemyCountMax;
         ++EnemyIndex)
    {
        enemy* Enemy = World->Enemies + EnemyIndex;
        if (!Enemy->Active)
        {
            Enemy->Active = true;

            Enemy->Health = Health;

            Enemy->BehaviorFunction = Behavior;

            v2 Position = V2(RandomRange(GameState, (real32)WorldSizeX),
                             RandomRange(GameState, (real32)WorldSizeY - 300.0f));
            v2 Velocity = V2(RandomPerturb(GameState, 0.0f, 20.0f),
                             RandomPerturb(GameState, 0.0f, 20.0f));
            InitializeSpatial_Linear(&Enemy->Spatial, Position, Velocity);
            InitializeCollisionArea_Circle(&Enemy->CollisionArea, Radius);
            Found = true;
            ++World->EnemyCount;
            break;
        }
    }

    Assert("No ununsed enemies in pool.", Found);
}

boss*
CreateBoss(game_state* GameState,
           real32 DamageRadius,
           real32 PlayerKillRadius)
{
    boss* Boss = GameState->World->Boss;

    Boss->AttackCount = 0;
    Boss->CurrentAttackIndex = 0;

    Boss->Active = true;

    InitializeSpatial_Linear(&Boss->Spatial,
                             V2(WorldSizeX / 2.0f, 100.0f),
                             V2(0.0f, 0.0f));
    InitializeCollisionArea_Circle(&Boss->DamageCollisionArea, DamageRadius);
    InitializeCollisionArea_Circle(&Boss->PlayerKillCollisionArea, PlayerKillRadius);

    return Boss;
}

void
AddBossAttack(boss* Boss, boss_behavior_function Behavior, int32 Health, int32 Duration)
{
    boss_attack* Attack = &Boss->Attacks[Boss->AttackCount];

    Attack->Behavior = Behavior;

    Attack->Duration = Duration;

    Attack->Health = Health;
    Attack->HealthRemaining = Health;

    ++Boss->AttackCount;
}

void
HitEnemy(game_mode_world* World, enemy* Enemy, int32 Damage)
{
    Enemy->Health -= Damage;
    if (Enemy->Health <= 0)
    {
        DestroyEnemy(World, Enemy);
        CreateExplosion2(World,
                         Enemy->Spatial.Position);
    }
}

void
DestroyBoss(game_mode_world* World)
{
    World->Boss->Active = false;
}

void
HitBoss(game_state* GameState, int32 Damage)
{
    game_mode_world* World = GameState->World;
    boss* Boss = World->Boss;

    //PlayRandomSound(GameState, SoundGroupId_Impact);
    CreateExplosion2(World,
                     Boss->Spatial.Position + V2(RandomPerturb(GameState, 0.0f, Boss->DamageCollisionArea.Radius),
                                                 RandomPerturb(GameState, 0.0f, Boss->DamageCollisionArea.Radius)));

    boss_attack* Attack = GetCurrentAttack(Boss);
    if (Attack)
    {
        Attack->HealthRemaining -= Damage;
    }
}

void
ActivateNextBossAttack(boss* Boss, int32 Time)
{
    ++Boss->CurrentAttackIndex;
    boss_attack* BossAttack = &Boss->Attacks[Boss->CurrentAttackIndex];
    // NOTE: Add one to start time, since this attack will only be updated next frame
    BossAttack->StartTime = Time + 1;
}

void
UpdateEnemies(game_state* GameState,
              int32 Time)
{
    game_mode_world* World = GameState->World;
    for (uint32 EnemyIndex = 0;
         EnemyIndex < EnemyCountMax;
         ++EnemyIndex)
    {
        enemy* Enemy = World->Enemies + EnemyIndex;
        if (Enemy->Active)
        {
            Enemy->BehaviorFunction(World, Enemy, Time);
            UpdateSpatial(&Enemy->Spatial);
        }

        for (int32 HealthIndex = 0;
             HealthIndex < Enemy->Health;
             ++HealthIndex)
        {
            PushRect(GameState->Renderer,
                     Rect(Enemy->Spatial.Position + V2(-10.0f + (HealthIndex * 7.0f), 0.0f),
                          V2(4.0f, 4.0f)),
                     Colors.Red,
                     true,
                     EngineLayer_EnemyDefault);


        }
    }

    boss* Boss = World->Boss;
    if (Boss->Active)
    {
        if (Boss->AttackCount > 0)
        {
            boss_attack* BossAttack = &Boss->Attacks[Boss->CurrentAttackIndex];

            int32 AttackTime = Time - BossAttack->StartTime;
            BossAttack->Behavior(GameState, Boss, AttackTime);
            UpdateSpatial(&Boss->Spatial);

            if (!World->FreezeBossTime)
            {
                BossAttack->TimeRemaining = BossAttack->Duration - AttackTime;
            }

            bool32 AttackFinished = false;
            if (BossAttack->HealthRemaining <= 0)
            {
                if (Boss->CurrentAttackIndex == Boss->AttackCount - 1)
                {
                    DestroyBoss(World);
                    ScreenShake(GameState->Renderer, 4.0f, 0.4f);
                    CreateExplosion(GameState,
                                    Boss->Spatial.Position,
                                    Boss->DamageCollisionArea.Radius,
                                    0.3f, 1.3f,
                                    30, 140,
                                    TextureId_Particle_Smoke,
                                    20);
                }
                else
                {
                    ActivateNextBossAttack(Boss, Time);
                }
                AttackFinished = true;
            }
            else if (BossAttack->TimeRemaining <= 0)
            {
                if (Boss->CurrentAttackIndex == Boss->AttackCount - 1)
                {
                    DestroyBoss(World);
                }
                else
                {
                    ActivateNextBossAttack(Boss, Time);
                }
                AttackFinished = true;
            }
            if (AttackFinished)
            {
                FadeProjectiles_Targeted(GameState, Target_Player);
            }
        }

        spatial* BossSpatial = &Boss->Spatial;
        PushTexture(GameState->Renderer,
                    TextureId_Enemy_Scarab,
                    GetFullTextureRect(GameState, TextureId_Enemy_Scarab),
                    BossSpatial->Position,
                    1.0f, EngineLayer_BossDefault);

        boss_attack* BossAttack = GetCurrentAttack(Boss);
        if (BossAttack)
        {
            real32 HealthPercentage = (real32)BossAttack->HealthRemaining / (real32)BossAttack->Health;
            int32 HealthWidth = RoundReal32(HealthPercentage * (WorldSizeX - 40.0f));
            PushRect(GameState->Renderer,
                     Rect(20.0f, 20.0f, HealthWidth, 4.0f),
                     Colors.White,
                     true,
                     EngineLayer_BossInfo);
            char TimeRemainingText[16];
            sprintf(TimeRemainingText, "%d", CeilReal32(FramesToTime(BossAttack->TimeRemaining)));
            PushText(GameState->Renderer,
                     TimeRemainingText,
                     V2(WorldSizeX - 32, 4),
                     Colors.White,
                     false,
                     EngineLayer_BossInfo);

        }
    }
}

v2
GetPosition(boss* Boss)
{
    return Boss->Spatial.Position;
}

void
SetPosition(boss* Boss, v2 Position)
{
    Boss->Spatial.Position = Position;
}

real32
GetStoredVariable(boss* Boss, uint32 Index)
{
    Assert("Storage slot out of bounds.", Index < ProjectileStorageSize);
    return Boss->StoredVariables[Index];
}

void
SetStoredVariable(boss* Boss, uint32 Index, real32 Value)
{
    Assert("Storage slot out of bounds.", Index < ProjectileStorageSize);
    Boss->StoredVariables[Index] = Value;
}

bool32
IsActive(boss* Boss)
{
    return Boss && Boss->Active;
}

boss*
GetBoss(game_state* GameState)
{
    // TODO: Safety...
    return GameState->World->Boss;
}
