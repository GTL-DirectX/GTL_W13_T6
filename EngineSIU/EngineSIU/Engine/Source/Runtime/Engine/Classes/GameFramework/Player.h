#pragma once
#include "Character.h"
#include "UObject/ObjectMacros.h"

class USpringArmComponent;
class UProjectileMovementComponent;
class UCameraComponent;
class UWeaponComponent;
class UAnimSequenceBase;

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
    virtual void OnDamaged(FVector KnockBackDir) override;
    void PlayDamageCameraShake();
    void PlayHitCameraShake();
    void VibrateMotorHit();

    float GetScore() const { return Score; }
private:
    void StartGame();

    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void UpdateFacingRotation(float DeltaTime);
    void MoveUp(float DeltaTime);

    void RotateYaw(float DeltaTime);
    void RotatePitch(float DeltaTime) const;

    void PlayerConnected(int TargetIndex) const;
    void PlayerDisconnected(int TargetIndex) const;

    void SetControllerVibration(float LeftMotor, float RightMotor) const;

    FName Socket = "jx_c_camera";
    
    UCameraComponent* CameraComponent = nullptr;
    USpringArmComponent* SpringArmComponent = nullptr;

    int PlayerIndex = -1;
    int TargetViewPlayer = -1;

    float Score;

public:
    void Stun() const;
    void KnockBack(FVector KnockBackDir) const;
    void OnDead() const;
    void Attack();

    void EquipWeapon(UWeaponComponent* WeaponComponent);
    void AttachSocket();

    void SetLinearSpeed(float InLinearSpeed) { LinearSpeed = InLinearSpeed; }
    float GetLinearSpeed() const { return LinearSpeed; }

private:
    void BindAnimNotifys();

    void OnStartAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
    void OnFinishAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);

protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Player.lua")
    UPROPERTY(EditAnywhere, FString, StateMachineFileName, = "LuaScripts/Animations/PlayerStateMachine.lua")


private:
    
    UWeaponComponent* EquippedWeapon = nullptr; // 현재 장착된 무기 컴포넌트
    
    float Acceleration = 100000.0f;
    float MaxSpeed = 100000.0f;
    float RawSpeed = 100.0f; // 좌우 회전 속도
    float PitchSpeed = 100.0f;

    bool hitted = false;
    float ElapsedMotorTime = 0.f;
    float MotorTime = 0.2f;
    float LinearSpeed;
};
