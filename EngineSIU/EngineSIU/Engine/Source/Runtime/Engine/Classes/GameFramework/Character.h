#pragma once

#include "Pawn.h"

class USkeletalMeshComponent;
class UCapsuleComponent;

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
    UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }
    
protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
    UCapsuleComponent* CapsuleComponent = nullptr;

public:
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
    // bool IsFalling() const;
    // bool IsJumping();
    // bool IsAttacking();


};
