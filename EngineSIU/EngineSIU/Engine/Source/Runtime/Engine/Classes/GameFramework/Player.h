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
    virtual void BeginPlay() override;
    
    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override;
    void SetPlayerIndex(int InPlayerIndex) { PlayerIndex = InPlayerIndex; TargetViewPlayer = InPlayerIndex; }

    int GetTargetViewPlayer() const { return TargetViewPlayer; }
    void SetTargetViewPlayer(int Index) { TargetViewPlayer = Index; }
    void ChangeTargetViewPlayer(int ChangeAmount);
    
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.
    
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
    int TargetViewPlayer = -1;

public:
    void OnDamaged(FVector KnockBackDir) const;
    void Stun() const;
    void KnockBack(FVector KnockBackDir) const;
    void Dead() const;
    void Attack() const;

    void EquipWeapon(UWeaponComponent* WeaponComponent);

protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Player.lua")
    UPROPERTY(EditAnywhere, FString, StateMachineFileName, = "LuaScripts/Animations/PlayerStateMachine.lua")


private:
    
    UWeaponComponent* EquippedWeapon = nullptr; // 현재 장착된 무기 컴포넌트
    
    FVector Velocity = FVector(); // 이동 속도
    float Acceleration = 100000.0f;
    float MaxSpeed = 100000.0f;
    float RawSpeed = 100.0f; // 좌우 회전 속도
    float PitchSpeed = 100.0f;
    
    float StunGauge = 0.0f;
    float MaxStunGauge = 100.0f;

    float KnockBackPower = 100000;
};
