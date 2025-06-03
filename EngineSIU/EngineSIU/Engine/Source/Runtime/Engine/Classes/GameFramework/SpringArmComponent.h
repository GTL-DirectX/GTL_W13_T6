#pragma once
#include "Components/SceneComponent.h"
#include <optional>

class UCameraComponent;
class USphereComponent;
struct FPointerEvent;

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
    USpringArmComponent();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime) override;
    void SetTargetArmLength(float InLength);
    float GetTargetArmLength() const;
    void SetSocketOffset(const FVector& InOffset);
    FVector GetSocketOffset() const;
    FVector GetCameraForwardVector();
    FVector GetCameraRightVector();

    void AttachCamera(UCameraComponent* InCamera);
    void DetachCamera();

    bool IsCameraAttached() const { return bIsCameraAttached; }

protected:
    void OnRawMouseInput(const FPointerEvent& InEvent);
    void HandleRotation(const FVector2D& Vector);
    void UpdateCameraTransform(float DeltaTime);

public:
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, TargetArmLength, =300.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, SocketOffset, = FVector(0, 50.0f, 50.0f))
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MinPitch, = -15.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MaxPitch, = 70.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MinYaw, = -70.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MaxYaw, = 70.f)

    FVector TargetLocation; // 최종 카메라 위치

    float CurrentPitchAngle = 0.0f;
    float CurrentYawAngle = 0.0f;

    UCameraComponent* Camera = nullptr;

    bool bIsCameraAttached = false; // 카메라가 Springarm에 붙어있는지 여부
    FVector CachedCameraLocation; // 카메라의 위치
    FRotator CachedCameraRotation; // 카메라의 회전

    // Collision은 안함
private:
    std::optional<FDelegateHandle> MouseInputHandle;
};
