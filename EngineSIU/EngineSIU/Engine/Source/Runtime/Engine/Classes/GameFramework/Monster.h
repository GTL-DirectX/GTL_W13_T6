#pragma once
#include "Character.h"
#include "Distribution/DistributionVector.h"

class UAnimSequenceBase;
class USkeletalMeshComponent;

class AMonster : public ACharacter
{
    DECLARE_CLASS(AMonster, ACharacter)
public:
    AMonster();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;
    virtual void AddMonsterAnimSequenceCache();
    virtual void AddAnimNotifies();

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;


public:
    // -- State Management -- //
    bool IsFalling() const;
    bool TestToggleVariable() const;
    void OnToggleLanding(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
    void OnToggleRoaring(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
    void OnToggleHit(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
    void SetFalling(bool bInFalling) { bFalling = bInFalling; }

    bool GetIsChasing() const { return bIsChasing; }
    void SetIsChasing(bool Value) { bIsChasing = Value; }

    bool IsFallingToDeath() const { return bFallingToDeath; }
    void SetFallingToDeath(bool b) { bFallingToDeath = b; }

    bool IsDead() const { return bDead; }
    void SetDead(bool b) { bDead = b; }

    bool IsHit() const { return bIsHit; }
    void SetHit(bool b) { bIsHit = b; }

    void SetIsLanding(bool bInLanding) { bIsLanding = bInLanding; }
    bool IsLanding();

    void SetIsRoaring(bool bInRoaring) { bIsRoaring = bInRoaring; }
    bool IsRoaring() const { return bIsRoaring; }

    // ---------------------- //
    float GetFollowTimer() const { return FollowTimer; }
    void SetFollowTimer(float Value) { FollowTimer = Value; }

    void OnDamaged(FVector KnockBackDir) const;

    FVector GetTargetPosition() const { return TargetPos; }
    void SetTargetPosition(const FVector NewTargetPos) { TargetPos = NewTargetPos; }
    void UpdateTargetPosition();


protected:
    TMap<FString, FString> StateToAnimName;
    TMap<FString, UAnimSequenceBase*> StateToAnimSequence;
    static inline bool bNotifyInitialized = false;

    UPROPERTY(EditAnywhere, FString, AnimPath, = "Contents/Bowser")
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Monster.lua")
    UPROPERTY(EditAnywhere, FString, StateMachineFileName, = "LuaScripts/Animations/MonsterStateMachine.lua")

    UPROPERTY(EditAnywhere, float, FollowTimer, = 0.0f)

    // -- State -- //
    UPROPERTY(EditAnywhere, bool, bIsHit, = false)
    UPROPERTY(EditAnywhere, bool, bDead, = false)
    UPROPERTY(EditAnywhere, bool, bFalling, = true)
    UPROPERTY(EditAnywhere, bool, bIsLanding, = false)
    UPROPERTY(EditAnywhere, bool, bIsChasing, = false)
    UPROPERTY(EditAnywhere, bool, bIsRoaring, = false)
    UPROPERTY(EditAnywhere, bool, bFallingToDeath, = false)

    UPROPERTY(EditAnywhere, bool, bLandEnded, = false)
    UPROPERTY(EditAnywhere, bool, bRoarEnded, = false)

    FDistributionVector TargetDistributionVector;
    FVector TargetPos;
};
