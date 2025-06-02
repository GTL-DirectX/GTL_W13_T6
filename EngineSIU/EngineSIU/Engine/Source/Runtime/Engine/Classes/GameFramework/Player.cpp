#include "Player.h"

#include "PhysicsManager.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
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
    NewActor->CameraComponent->SetRelativeLocation(FVector(3,0,9));
    // TODO: 미리 만들어둔 Player Duplicate 할 때 Component들 복제 필요한 애들 복제해주기
    
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);

    CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    CameraComponent->SetRelativeLocation(FVector(3,0,9));
    CameraComponent->SetupAttachment(RootComponent);

    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Player_3TTook/Player_Running")));
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);
    
    SetActorLocation(FVector(10, 10, 0) * PlayerIndex + FVector(0,0,30));
    SetActorScale(FVector(0.05));
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    MoveSpeed = Velocity.Length();
    CapsuleComponent->BodyInstance->AddForce(Velocity);
    
    // if (SkeletalMeshComponent)
    // {
    //     const FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(Socket);
    //     SetActorRotation(SocketTransform.GetRotation().Rotator());
    //     SetActorLocation(SocketTransform.GetTranslation());
    // }
}

void APlayer::BeginPlay()
{
    Super::BeginPlay();

    if (SkeletalMeshComponent)
    {
        if (ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(SkeletalMeshComponent->GetAnimInstance()))
        {
            if (auto StateMachine = AnimInstance->GetAnimStateMachine())
            {
                StateMachine->BindTargetActor(this);
            }
        }
    }
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

        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_A, [this](float DeltaTime) { OnDamaged(FVector(-1, 0, 0)); }); // 테스트 코드
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_B, [this](float DeltaTime) { Attack(); });

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
    if (PlayerState >= EPlayerState::Attacking)
    {
        return;
    }
    
    Velocity += GetActorForwardVector() * Acceleration * DeltaTime;
    
    if (MoveSpeed > MaxSpeed)
    {
        MoveSpeed = MaxSpeed;
        Velocity.Normalize() * MaxSpeed;
    }
}

void APlayer::MoveRight(float DeltaTime)
{
    if (PlayerState >= EPlayerState::Attacking)
    {
        return;
    }
    
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
    if (!CapsuleComponent)
    {
        return;
    }

    FBodyInstance* BodyInstance = CapsuleComponent->BodyInstance;
    if (!BodyInstance)
    {
        return;
    }

    if (PxRigidDynamic* RigidActor = BodyInstance->BIGameObject->DynamicRigidBody)
    {
        // 현재 Transform 가져오기
        PxTransform CurrentTransform = RigidActor->getGlobalPose();
    
        // 회전할 각도 계산 (Yaw)
        float YawRadians = FMath::DegreesToRadians(RawSpeed * DeltaTime);
    
        // Z축 기준 회전 쿼터니언 생성
        PxQuat YawRotation(YawRadians, PxVec3(0.0f, 0.0f, 1.0f));
    
        // 현재 회전에 새로운 회전 적용
        CurrentTransform.q = CurrentTransform.q * YawRotation;
    
        // 새로운 Transform 설정
        RigidActor->setGlobalPose(CurrentTransform);
    }
}

void APlayer::RotatePitch(float DeltaTime) const
{
    if (CameraComponent)
    {
        FRotator NewRotation = CameraComponent->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaTime * PitchSpeed, -89.0f, 89.0f);
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
    DEFINE_LUA_TYPE_WITH_PARENT(APlayer, sol::bases<AActor, APawn, ACharacter>(),
    "MoveSpeed", &APlayer::MoveSpeed,
    "Velocity", &APlayer::Velocity,
    "Acceleration", &APlayer::Acceleration,
    "MaxSpeed", &APlayer::MaxSpeed,
    "RawSpeed", &APlayer::RawSpeed,
    "PitchSpeed", &APlayer::PitchSpeed,
    "StunGauge", &APlayer::StunGauge,
    "MaxStunGauge", &APlayer::MaxStunGauge,
    "KnockBackPower", &APlayer::KnockBackPower,
    "State", sol::property(&APlayer::GetState, &APlayer::SetState)
    )
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
    
    return true;
}

void APlayer::OnDamaged(FVector KnockBackDir) const
{
    LuaScriptComponent->ActivateFunction("OnDamaged", KnockBackDir);
}

void APlayer::Stun() const
{
    LuaScriptComponent->ActivateFunction("Stun");
}

void APlayer::KnockBack(FVector KnockBackDir) const
{
    LuaScriptComponent->ActivateFunction("KnockBack", KnockBackDir);
    
}

void APlayer::Dead() const
{
    LuaScriptComponent->ActivateFunction("Dead");
}

void APlayer::Attack() const
{
    LuaScriptComponent->ActivateFunction("Attack");
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
