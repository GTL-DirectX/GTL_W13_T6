#include "Player.h"

#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "World/World.h"

#include "Engine/Contents/Weapons/Weapon.h"
#include "Engine/Contents/Weapons/WeaponComponent.h"

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->Socket = Socket;
    NewActor->CameraComponent = Cast<UCameraComponent>(CameraComponent->Duplicate(NewActor));
    NewActor->CameraComponent->SetRelativeLocation(FVector(2,0,0));
    
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    CameraComponent = AddComponent<UCameraComponent>();
    CameraComponent->SetupAttachment(RootComponent);
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // if (SkeletalMeshComponent)
    // {
    //     const FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(Socket);
    //     SetActorRotation(SocketTransform.GetRotation().Rotator());
    //     SetActorLocation(SocketTransform.GetTranslation());
    // }
}

void APlayer::SetupInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupInputComponent(PlayerInputComponent);
    // 카메라 조작용 축 바인딩
    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindAction("W", [this](float DeltaTime) { MoveForward(DeltaTime); });
        PlayerInputComponent->BindAction("S", [this](float DeltaTime) { MoveForward(-DeltaTime); });
        PlayerInputComponent->BindAction("A", [this](float DeltaTime) { MoveRight(-DeltaTime); });
        PlayerInputComponent->BindAction("D", [this](float DeltaTime) { MoveRight(DeltaTime); });
        PlayerInputComponent->BindAction("E", [this](float DeltaTime) { MoveUp(DeltaTime); });
        PlayerInputComponent->BindAction("Q", [this](float DeltaTime) { MoveUp(-DeltaTime); });

        PlayerInputComponent->BindAxis("Turn", [this](float DeltaTime) { RotateYaw(DeltaTime); });
        PlayerInputComponent->BindAxis("LookUp", [this](float DeltaTime) { RotatePitch(DeltaTime); });

        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_A, [this](float DeltaTime) { MoveUp(DeltaTime); });
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_B, [this](float DeltaTime) { MoveUp(-DeltaTime); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickY, [this](float DeltaTime) { MoveForward(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickX, [this](float DeltaTime) { MoveRight(DeltaTime); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickX, [this](float DeltaTime) { RotateYaw(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickY, [this](float DeltaTime) { RotatePitch(DeltaTime); });

        PlayerInputComponent->BindControllerConnected(PlayerIndex, [this](int Index){ PlayerConnected(Index); });
        PlayerInputComponent->BindControllerDisconnected(PlayerIndex, [this](int Index){ PlayerDisconnected(Index); });
    }
}

void APlayer::MoveForward(float DeltaTime)
{
    FVector Delta = GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::MoveRight(float DeltaTime)
{
    FVector Delta = GetActorRightVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::MoveUp(float DeltaTime)
{
    FVector Delta = GetActorUpVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::RotateYaw(float DeltaTime)
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += DeltaTime * RotationSpeed; // Yaw 회전 속도
    SetActorRotation(NewRotation);
}

void APlayer::RotatePitch(float DeltaTime) const
{
    if (CameraComponent)
    {
        FRotator NewRotation = CameraComponent->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaTime * RotationSpeed, -89.0f, 89.0f);
        CameraComponent->SetRelativeRotation(NewRotation);
    }
}

void APlayer::PlayerConnected(int TargetIndex) const
{
    if (TargetIndex == PlayerIndex)
    {
        GetWorld()->ConnectedPlayer(TargetIndex);
    }
}

void APlayer::PlayerDisconnected(int TargetIndex) const
{
    if (TargetIndex == PlayerIndex)
    {
        GetWorld()->DisconnectedPlayer(TargetIndex);
    }
}

void APlayer::Attack()
{
    if (!EquippedWeapon)
    {
        return;
    }

    EquippedWeapon->Attack();
}

void APlayer::EquipWeapon(UWeaponComponent* WeaponComponent)
{
    if (!WeaponComponent || !EquippedWeapon)
    {
        return;
    }

    if (EquippedWeapon)
    {
        // 이미 장착된 무기가 있다면 기존 무기를 제거
        EquippedWeapon->DestroyComponent();
        EquippedWeapon = nullptr;
    }

    EquippedWeapon = WeaponComponent;
    EquippedWeapon->SetOwner(this);

    // 무기 컴포넌트가 장착되면 애니메이션 블루프린트에 연결

}
