#pragma once
#include "Character.h"
#include "UObject/ObjectMacros.h"

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
    
    float MoveSpeed = 100.0f; // 이동 속도
    float RotationSpeed = 100.0f; // 회전 속도

public:
    void Attack();
    void EquipWeapon(UWeaponComponent* WeaponComponent);

private:
    UWeaponComponent* EquippedWeapon = nullptr; // 현재 장착된 무기 컴포넌트
};
