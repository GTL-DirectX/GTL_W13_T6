#include "Player.h"

#include "PhysicsManager.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "World/World.h"

#include "Engine/Contents/Weapons/Weapon.h"
#include "Engine/Contents/Weapons/MeleeWeaponComponent.h"

#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Engine/FObjLoader.h"
#include "sol/sol.hpp"

#include "Animation/AnimSequence.h"

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->Socket = Socket;
    NewActor->CameraComponent = Cast<UCameraComponent>(CameraComponent->Duplicate(NewActor));
    NewActor->CameraComponent->SetRelativeLocation(FVector(-10,0,9));
    // TODO: 미리 만들어둔 Player Duplicate 할 때 Component들 복제 필요한 애들 복제해주기
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);

    CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    CameraComponent->SetRelativeLocation(FVector(-20,0,20));
    CameraComponent->SetRelativeRotation(FRotator(-40,0,0));
    CameraComponent->SetupAttachment(RootComponent);

    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Player_3TTook/Player_Running")));
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);

   
    SetActorLocation(FVector(10, 10, 0) * PlayerIndex + FVector(0, 0, 30));
    AttachSocket();
    
   
    BindAnimNotifys();
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FBodyInstance* BodyInstance = CollisionComponent->BodyInstance;
    if (!BodyInstance)
    {
        return;
    }

    if (PxRigidDynamic* RigidActor = BodyInstance->BIGameObject->DynamicRigidBody)
    {
        LinearSpeed = RigidActor->getLinearVelocity().magnitude();
        UE_LOG(ELogLevel::Error, TEXT("Linear Speed: %f"), LinearSpeed);
    }
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

        PlayerInputComponent->BindAction("P", [this](float DeltaTime) { Attack(); }); // 공격 액션 바인딩

        PlayerInputComponent->BindAxis("Turn", [this](float DeltaTime) { RotateYaw(DeltaTime * 0.01f); });
        PlayerInputComponent->BindAxis("LookUp", [this](float DeltaTime) { RotatePitch(DeltaTime); });

        // PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_A, [this](float DeltaTime) { OnDamaged(FVector(-1, 0, 0)); }); // 테스트 코드
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_B, [this](float DeltaTime) { Attack(); });

        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_LEFT_SHOULDER, [this](float Temp) { ChangeTargetViewPlayer(1); });
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, [this](float Temp) { ChangeTargetViewPlayer(1); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickY, [this](float DeltaTime) { MoveForward(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickX, [this](float DeltaTime) { MoveRight(DeltaTime); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickX, [this](float DeltaTime) { RotateYaw(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickY, [this](float DeltaTime) { RotatePitch(DeltaTime); });

        PlayerInputComponent->BindControllerConnected(PlayerIndex, [this](int Index) { PlayerConnected(Index); });
        PlayerInputComponent->BindControllerDisconnected(PlayerIndex, [this](int Index) { PlayerDisconnected(Index); });
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
        Velocity.Normalize()* MaxSpeed;
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
        Velocity.Normalize()* MaxSpeed;
    }
}

void APlayer::MoveUp(float DeltaTime)
{
    FVector Delta = GetActorUpVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::RotateYaw(float DeltaTime)
{
    if (!CollisionComponent)
    {
        return;
    }

    FBodyInstance* BodyInstance = CollisionComponent->BodyInstance;
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

void APlayer::ChangeTargetViewPlayer(int ChangeAmount)
{
    TargetViewPlayer += ChangeAmount;
    TargetViewPlayer %= 4;
    while (true) // 죽은 플레이어 건너뛰기
    {
        APlayer* TargetPlayer = GetWorld()->GetPlayer(TargetViewPlayer);
        if (TargetPlayer && TargetPlayer->GetState() != static_cast<int>(EPlayerState::Dead))
        {
            break;
        }
        
        TargetViewPlayer += ChangeAmount;
        TargetViewPlayer %= 4;
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
    "Acceleration", &APlayer::Acceleration,
    "MaxSpeed", &APlayer::MaxSpeed,
    "RawSpeed", &APlayer::RawSpeed,
    "PitchSpeed", &APlayer::PitchSpeed,
    "ChangeViewTarget", &APlayer::ChangeTargetViewPlayer,
        "LinearSpeed", sol::property(&APlayer::GetLinearSpeed, &APlayer::SetLinearSpeed)
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
    // AttachSocket();
     // 무기 컴포넌트가 장착되면 애니메이션 블루프린트에 연결
}

/*
* 현재 StaticMeshComp는 소캣 테스트를 위한 임시 컴포넌트, 
*
* 무기 Overlapped 구현됐을 때 StaticMeshComp대신 EquippedWeapon 사용하면 됨
* 소캣 위치는 현재 Glove 기준. 프라이팬 기준으로 수정 필요함
*/
void APlayer::AttachSocket()
{
    if (EquippedWeapon = AddComponent<UMeleeWeaponComponent>())
    {
        FVector Pos = FVector(2.4f, -5.1, 40.3);
        FRotator Rot = FRotator(178, -178, 13);
        FVector Scale = FVector(5, 5, 5);
        FTransform TF = FTransform(Rot, Pos, Scale);
        SkeletalMeshComponent->AddSocket("LeftHand", "mixamorig:LeftHand", TF);

        EquippedWeapon->SetupAttachment(SkeletalMeshComponent);

        EquippedWeapon->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/PUBG/FlyPan.obj"));

        EquippedWeapon->SetAttachSocketName("LeftHand");
    }
}

void APlayer::BindAnimNotifys()
{
    UAnimSequenceBase* AttackAnim = Cast<UAnimSequenceBase>(UAssetManager::Get().GetAnimation(FName("Contents/Player_3TTook/Armature|Armature|Armature|Left_Hook")));
    int32 TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(AttackAnim, FName(TEXT("Default")));
    if (TrackIdx == INDEX_NONE)
    {
        return;
    }
    int32 NewNotifyIndex = INDEX_NONE;
    float NotifyTime = 0.1f;
    bool bAdded = AttackAnim->AddDelegateNotifyEventAndBind<APlayer>(TrackIdx, NotifyTime, this, &APlayer::OnStartAttack, NewNotifyIndex);
   
    NewNotifyIndex = INDEX_NONE;
    NotifyTime = 0.9f;
    bAdded = AttackAnim->AddDelegateNotifyEventAndBind<APlayer>(TrackIdx, NotifyTime, this, &APlayer::OnFinishAttack, NewNotifyIndex);

}

void APlayer::OnStartAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    /*if (EquippedWeapon)
    {
        EquippedWeapon->Attack();
    }*/
}

void APlayer::OnFinishAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (EquippedWeapon)
    {
        EquippedWeapon->FinishAttack();
    }
}
