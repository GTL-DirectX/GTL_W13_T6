#pragma once
#include "Character.h"
#include "Distribution/DistributionVector.h"

class USkeletalMeshComponent;

class AMonster : public ACharacter
{
    DECLARE_CLASS(AMonster, ACharacter)
public:
    AMonster() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // -- State Management -- //
    bool IsFalling() const;
    void SetFalling(bool bInFalling) { bFalling = bInFalling; }

    bool GetIsChasing() const { return bIsChasing; }
    void SetIsChasing(bool Value) { bIsChasing = Value; }

    bool IsFallingToDeath() const { return bFallingToDeath; }
    void SetFallingToDeath(bool b) { bFallingToDeath = b; }

    bool IsDead() const { return bDead; }
    void SetDead(bool b) { bDead = b; }

    bool IsHit() const { return bHit; }
    void SetHit(bool b) { bHit = b; }

    // ---------------------- //
    float GetFollowTimer() const { return FollowTimer; }
    void SetFollowTimer(float Value) { FollowTimer = Value; }


    FVector GetTargetPosition() const { return TargetPos; }
    void SetTargetPosition(const FVector NewTargetPos) { TargetPos = NewTargetPos; }
    void UpdateTargetPosition();


protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Monster.lua")
    UPROPERTY(EditAnywhere, FString, StateMachineFileName, = "LuaScripts/Animations/MonsterStateMachine.lua")

    UPROPERTY(EditAnywhere, float, FollowTimer, = 0.0f)

    // -- State -- //
    UPROPERTY(EditAnywhere, bool, bHit, = false)
    UPROPERTY(EditAnywhere, bool, bDead, = false)
    UPROPERTY(EditAnywhere, bool, bFalling, = false)
    UPROPERTY(EditAnywhere, bool, bIsChasing, = false)
    UPROPERTY(EditAnywhere, bool, bFallingToDeath, = false)

    FDistributionVector TargetDistributionVector;
    FVector TargetPos;
};
