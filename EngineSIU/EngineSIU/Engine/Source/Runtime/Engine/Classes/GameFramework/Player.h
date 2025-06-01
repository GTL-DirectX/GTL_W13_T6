#pragma once
#include "Character.h"
#include "UObject/ObjectMacros.h"

class UProjectileMovementComponent;
class UCameraComponent;
class UWeaponComponent;

class APlayer : public ACharacter
{
    DECLARE_CLASS(APlayer, ACharacter)
public:
    APlayer() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override;
    
    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override;
    void SetPlayerIndex(int InPlayerIndex) { PlayerIndex = InPlayerIndex; }
    
private:
    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void MoveUp(float DeltaTime);

    void RotateYaw(float DeltaTime);
    void RotatePitch(float DeltaTime) const;

    void PlayerConnected(int TargetIndex) const;
    void PlayerDisconnected(int TargetIndex) const;

    FName Socket = "jx_c_camera";
    
    UCameraComponent* CameraComponent = nullptr;

    int PlayerIndex = -1;
    
    FVector GetVelocity() const { return Velocity; }
    float GetAcceleration() const { return Acceleration; }
    void SetAcceleration(float NewAcceleration) { Acceleration = NewAcceleration; }
    float GetMaxSpeed() const { return MaxSpeed; }
    void SetMaxSpeed(float NewMaxSpeed) { MaxSpeed = NewMaxSpeed; }
    float GetRotationSpeed() const { return RotationSpeed; }
    void SetRotationSpeed(float NewRotationSpeed) { RotationSpeed = NewRotationSpeed; }
public:
    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;
    
    void Attack();
    void EquipWeapon(UWeaponComponent* WeaponComponent);

protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Player.lua")

private:
    UWeaponComponent* EquippedWeapon = nullptr; // 현재 장착된 무기 컴포넌트
    
    FVector Velocity = FVector(); // 이동 속도
    float Acceleration = 10.0f;
    float MaxSpeed = 100.0f;
    float RotationSpeed = 100.0f; // 회전 속도
};
