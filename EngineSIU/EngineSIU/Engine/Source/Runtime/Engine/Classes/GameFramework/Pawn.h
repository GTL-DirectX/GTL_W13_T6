#pragma once
#include "Actor.h"

//TODO: Controller 부착 필요, 아직 Pawn은 쓰지 말 것

class UInputComponent;
class APlayerController;
class AController;

class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)
public:
    APawn() = default;
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    /** Pawn을 Controller에 의해 점유(Possess)될 때 호출 */
    virtual void PossessedBy(AController* NewController);

    /** Pawn이 Controller에서 해제(UnPossess)될 때 호출 */
    virtual void UnPossessed();

    /** 플레이어 입력을 수신하고 처리 */
    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent);

    /** 입력 처리용 함수 */
    virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f);

    /** 입력을 받아 회전 처리 */
    virtual void AddControllerYawInput(float Value);
    virtual void AddControllerPitchInput(float Value);

    /** 시야 관련 함수 */
    virtual FVector GetPawnViewLocation() const;
    virtual FRotator GetViewRotation() const;

    virtual void EnableInput(APlayerController* PlayerController);
    virtual void DisableInput(APlayerController* PlayerController);

    AController* GetController() const { return Controller; }

    virtual void RegisterLuaType(sol::state& Lua) override;

    void SetMoveSpeed(float InMoveSpeed) { MoveSpeed = InMoveSpeed; }
    float GetMoveSpeed() const
    {
        //UE_LOG(ELogLevel::Error, TEXT("MoveSpeed : %f "), MoveSpeed);
        return MoveSpeed;
    }

protected:
    UInputComponent* InputComponent;
    AController* Controller = nullptr;
    
    FVector PendingMovement;
    float MoveSpeed;
};
