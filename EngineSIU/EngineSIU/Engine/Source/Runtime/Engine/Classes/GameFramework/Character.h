#pragma once

#include "Pawn.h"

class USkeletalMeshComponent;
class UBoxComponent;
class UCapsuleComponent;
class UParticleSystemComponent;

enum class EPlayerState : uint8
{
    Idle = 0,
    Running, // 1
    Walking, // 2
    Attacking, // 3
    Stun, // 4
    MuJuck, // 5
    Dead, // 6
    Jumping, // 7
    Max, 
};

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)
public:
    ACharacter() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void PostSpawnInitialize() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override { }

    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
    UBoxComponent* GetCapsuleComponent() const { return CollisionComponent; }
    
protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
    UBoxComponent* CollisionComponent = nullptr;
    TMap<FString, UParticleSystemComponent*> ParticleSystemComponentMap;
    UCapsuleComponent* CapsuleComponent = nullptr;

public:
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
    void SetState(int State);
    int GetState();
    // bool IsFalling() const;
    // bool IsJumping();
    // bool IsAttacking();

    virtual void OnDamaged(FVector KnockBackDir);


protected:
    EPlayerState PlayerState = EPlayerState::Idle;

protected:
    void Jump();
    bool CheckGrounded();

protected:
    bool bIsGrounded;

    FVector Velocity = FVector(); // 이동 속도

    float StunGauge = 0.0f;
    float MaxStunGauge = 100.0f;

    float KnockBackPower = 100000;

};
