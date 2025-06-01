#include "Player.h"

#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "World/World.h"

#include "Engine/Contents/Weapons/Weapon.h"
#include "Engine/Contents/Weapons/WeaponComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "sol/sol.hpp"

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->Socket = Socket;
    NewActor->CameraComponent = Cast<UCameraComponent>(CameraComponent->Duplicate(NewActor));
    // TODO: 미리 만들어둔 Player Duplicate 할 때 Component들 복제 필요한 애들 복제해주기
    
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);

    CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    CameraComponent->SetRelativeLocation(FVector(2,0,0));
    CameraComponent->SetupAttachment(RootComponent);

    USkeletalMesh* SkeletalMeshAsset = UAssetManager::Get().GetSkeletalMesh("Contents/Human/Human");
    SkeletalMeshComponent->SetSkeletalMeshAsset(SkeletalMeshAsset);
    
    SetActorLocation(FVector(10, 10, 0) * PlayerIndex);
    SetActorScale(FVector(0.05));
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveSpeed = Velocity.Length();
    SetActorLocation(GetActorLocation() + Velocity);
    Velocity *= 0.8f;

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
    Velocity += GetActorForwardVector() * Acceleration * DeltaTime;
    
    if (MoveSpeed > MaxSpeed)
    {
        MoveSpeed = MaxSpeed;
        Velocity.Normalize() * MaxSpeed;
    }
}

void APlayer::MoveRight(float DeltaTime)
{
    Velocity += GetActorRightVector() * Acceleration * DeltaTime;
    
    if (MoveSpeed > MaxSpeed)
    {
        MoveSpeed = MaxSpeed;
        Velocity.Normalize() * MaxSpeed;
    }
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

void APlayer::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayer, sol::bases<AActor, ACharacter>(),
        "Velocity", sol::property(&ThisClass::GetVelocity),
        "Acceleration", sol::property(&ThisClass::GetAcceleration, &ThisClass::SetAcceleration),
        "MaxSpeed", sol::property(&ThisClass::GetMaxSpeed, &ThisClass::SetMaxSpeed),
        "RotationSpeed", sol::property(&ThisClass::GetRotationSpeed, &ThisClass::SetRotationSpeed)
   )
   // "Destroy", &ThisClass::Destroy
}

bool APlayer::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
    {
        return false;
    }

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    // LuaTable["MoveSpeed"] = MoveSpeed;
    // LuaTable["Acceleration"] = Acceleration;
    // LuaTable["MaxSpeed"] = MaxSpeed;
    // LuaTable["RotationSpeed"] = RotationSpeed;
    
    return true;
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
