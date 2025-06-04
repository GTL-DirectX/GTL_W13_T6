#include "Player.h"

#include "GameMode.h"
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
#include "SpringArmComponent.h"

#include "GameFramework/GameMode.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Contents/Objects/DamageCameraShake.h"

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->Socket = Socket;
    NewActor->CameraComponent = Cast<UCameraComponent>(CameraComponent->Duplicate(NewActor));
    NewActor->CameraComponent->SetRelativeLocation(FVector(-10, 0, 9));
    // TODO: 미리 만들어둔 Player Duplicate 할 때 Component들 복제 필요한 애들 복제해주기
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);

    // SpringArmComponent = AddComponent<USpringArmComponent>("SpringArmComponent");
    // SpringArmComponent->SetTargetArmLength(37.8f);
    // SpringArmComponent->SetSocketOffset(FVector(0, 14, 25.3));
    // SpringArmComponent->SetupAttachment(RootComponent);
    // CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    // CameraComponent->SetupAttachment(SpringArmComponent);



    CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    CameraComponent->SetRelativeLocation(FVector(-20,0,20));
    CameraComponent->SetRelativeRotation(FRotator(-40,0,0));
    CameraComponent->SetupAttachment(RootComponent);
    
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Player_3TTook/Player_Running")));
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);

    AttachSocket();
    
    BindAnimNotifys(); 
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PlayerState != EPlayerState::Dead)
    {
        Score = GetWorld()->GetGameMode()->GetGameInfo().ElapsedGameTime;
    }
    
    FBodyInstance* BodyInstance = CollisionComponent->BodyInstance;
    if (!BodyInstance)
    {
        return;
    }

    
    if (PxRigidDynamic* RigidActor = BodyInstance->BIGameObject->DynamicRigidBody)
    {
        LinearSpeed = RigidActor->getLinearVelocity().magnitude();
        UE_LOG(ELogLevel::Error, TEXT("Linear Speed: %f"), LinearSpeed);
        UE_LOG(ELogLevel::Error, TEXT("Velocity : %f"), Velocity);

        RigidActor->setAngularDamping(10.0f);
        bool bIsMovingInput = !Velocity.IsNearlyZero(1e-3f) && PlayerState != EPlayerState::MuJuck && bIsGrounded;
        //if (bIsMovingInput)
        //{
        //    // 플레이어가 입력으로 움직이고 있다면 낮은 감쇠 (즉, 관성 유지)
        //    RigidActor->setLinearDamping(0.1f);
        //}
        //else
        //{
        //    // 플레이어가 입력을 안 해서 멈춰 있거나 거의 멈춰 있으면 높은 감쇠
        //    RigidActor->setLinearDamping(100.0f);
        //}
        //UE_LOG(ELogLevel::Error, TEXT("Linear Speed: %f"), LinearSpeed);
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
        if (PlayerIndex == 0)
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
        }

        // PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_A, [this](float DeltaTime) { OnDamaged(FVector(-1, 0, 0)); }); // 테스트 코드
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_B, [this](float DeltaTime) { Attack(); });

        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_LEFT_SHOULDER, [this](float Temp) { ChangeTargetViewPlayer(1); });
        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, [this](float Temp) { ChangeTargetViewPlayer(1); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickY, [this](float DeltaTime) { MoveForward(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::LeftStickX, [this](float DeltaTime) { MoveRight(DeltaTime); });

        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickX, [this](float DeltaTime) { RotateYaw(DeltaTime); });
        PlayerInputComponent->BindControllerAnalog(EXboxAnalog::Type::RightStickY, [this](float DeltaTime) { RotatePitch(DeltaTime); });

        PlayerInputComponent->BindControllerButton(XINPUT_GAMEPAD_START, [this](float) { StartGame(); });

        PlayerInputComponent->BindControllerConnected(PlayerIndex, [this](int Index) { PlayerConnected(Index); });
        PlayerInputComponent->BindControllerDisconnected(PlayerIndex, [this](int Index) { PlayerDisconnected(Index); });
    }
}

void APlayer::MoveForward(float DeltaTime)
{
    if (PlayerState >= EPlayerState::Attacking || !GetCapsuleComponent())
    {
        return;
    }

    FBodyInstance* BodyInstance = GetCapsuleComponent()->BodyInstance;
    if (!BodyInstance || !BodyInstance->BIGameObject || !BodyInstance->BIGameObject->DynamicRigidBody)
    {
        return;
    }

    PxRigidDynamic* RigidBody = BodyInstance->BIGameObject->DynamicRigidBody;

    // PhysX → Unreal 변환
    const PxTransform PhysXTransform = RigidBody->getGlobalPose();
    const FQuat RotationQuat = FQuat(PhysXTransform.q.x, PhysXTransform.q.y, PhysXTransform.q.z, PhysXTransform.q.w);
    FVector ForwardVector = RotationQuat.GetForwardVector();

    // Velocity += ForwardVector * Acceleration * DeltaTime;
    Velocity += GetActorForwardVector() * Acceleration * DeltaTime;

    if (MoveSpeed > MaxSpeed)
    {
        MoveSpeed = MaxSpeed;
        Velocity = Velocity.GetSafeNormal() * MaxSpeed;
    }

    //UpdateFacingRotation(DeltaTime);
    //if (PlayerState >= EPlayerState::Attacking)
    //{
    //    return;
    //}
    //
    ////Velocity += GetActorForwardVector() * Acceleration * DeltaTime;
    //Velocity += GetActorForwardVector() * Acceleration * DeltaTime;

    //if (MoveSpeed > MaxSpeed)
    //{
    //    MoveSpeed = MaxSpeed;
    //    Velocity.Normalize()* MaxSpeed;
    //}
    //UpdateFacingRotation(DeltaTime);
}

void APlayer::MoveRight(float DeltaTime)
{
    // if (PlayerState >= EPlayerState::Attacking || !GetCapsuleComponent())
    // {
    //     return;
    // }
    //
    // FBodyInstance* BodyInstance = GetCapsuleComponent()->BodyInstance;
    // if (!BodyInstance || !BodyInstance->BIGameObject || !BodyInstance->BIGameObject->DynamicRigidBody)
    // {
    //     return;
    // }

    // PxRigidDynamic* RigidBody = BodyInstance->BIGameObject->DynamicRigidBody;
    //
    // // PhysX → Unreal 회전 정보
    // const PxTransform PhysXTransform = RigidBody->getGlobalPose();
    // const FQuat RotationQuat = FQuat(PhysXTransform.q.x, PhysXTransform.q.y, PhysXTransform.q.z, PhysXTransform.q.w);
    // FVector RightVector = RotationQuat.GetRightVector(); // 오른쪽 방향 추출
    //
    // Velocity += RightVector * Acceleration * DeltaTime;

    if (MoveSpeed > MaxSpeed)
    {
        MoveSpeed = MaxSpeed;
        Velocity = Velocity.GetSafeNormal() * MaxSpeed;
    }

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
    //UpdateFacingRotation(DeltaTime);
}
void APlayer::UpdateFacingRotation(float DeltaTime)
{
    // 1) BodyInstance와 PxRigidDynamic 확인
    if (!GetCapsuleComponent() || !GetCapsuleComponent()->BodyInstance)
    {
        return;
    }
    PxRigidDynamic* RigidActor = GetCapsuleComponent()
        ->BodyInstance
        ->BIGameObject
        ->DynamicRigidBody;
    if (!RigidActor)
    {
        return;
    }

    // 2) PhysX 속도(PxVec3) --> FVector로 변환
    PxVec3 PxVelocity = RigidActor->getLinearVelocity();
    FVector Velocity = FVector(PxVelocity.x, PxVelocity.y, PxVelocity.z);

    // 3) XY 평면 성분만 취하고, 너무 작은 속도는 무시
    FVector FlatVel = Velocity;
    FlatVel.Z = 0.f;
    if (FlatVel.IsNearlyZero(1e-3f))
    {
        return;
    }

    // 4) 목표 Yaw 계산 (FlatVel의 방향)
    float TargetYawRaw = FlatVel.Rotation().Yaw;
    //    - FlatVel.Rotation().Yaw는 일반적으로 -180~+180 범위

    // 5) 현재 Actor의 Yaw 가져오기 (역시 -180~+180 범위일 수 있음)
    float CurrentYawRaw = GetActorRotation().Yaw;

    // 6) 두 Yaw를 –180~+180° 범위로 정규화(Unwind)
    float CurrentYaw = FMath::UnwindDegrees(CurrentYawRaw);
    float TargetYaw = FMath::UnwindDegrees(TargetYawRaw);

    // 7) 두 Yaw 사이의 최소 각도 차이(Shortest Angle) 구하기
    //    ex) CurrentYaw= 179°, TargetYaw= -179° → DeltaYaw= -2°
    float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw);

    // 8) 차이가 너무 작으면 무시
    const float AngleThreshold = 1.0f; // 1° 미만 변화는 건너뜀
    if (FMath::Abs(DeltaYaw) < AngleThreshold)
    {
        return;
    }

    // 9) “초당 몇 도”만큼만 회전할 것인지 정의 (예: 180°/초)
    const float TurnSpeedDegPerSec = 180.0f;
    float MaxDeltaThisFrame = TurnSpeedDegPerSec * DeltaTime;

    // 10) 실제 회전량 = DeltaYaw를 [-MaxDeltaThisFrame, +MaxDeltaThisFrame]로 클램프
    float YawChange = FMath::Clamp(DeltaYaw, -MaxDeltaThisFrame, +MaxDeltaThisFrame);

    // 11) 새로운 Yaw = (CurrentYaw + YawChange), 이 상태도 –180~+180 범위를 약간 벗어날 수 있음
    float NewYawUnwound = CurrentYaw - YawChange;

    // 12) 다시 –180~+180°로 정규화하여 Actor에 적용
    float NewYaw = FMath::UnwindDegrees(NewYawUnwound);

    // 13) Actor 레벨에서 Yaw만 업데이트 (Pitch/Roll은 0)
    FRotator NewRotation(0.f, NewYaw, 0.f);
    SetActorRotation(NewRotation);

    // 14) PhysX Body에도 최소한의 회전만 반영
    PxTransform PxCurrentTM = RigidActor->getGlobalPose();
    FQuat    NewQuat = NewRotation.Quaternion();
    PxQuat   PxNewQuat(
        NewQuat.X,
        NewQuat.Y,
        NewQuat.Z,
        NewQuat.W
    );
    PxCurrentTM.q = PxNewQuat;
    RigidActor->setGlobalPose(PxCurrentTM, false);
}
//void APlayer::UpdateFacingRotation(float DeltaTime)
//{
//    // 1) BodyInstance와 PxRigidDynamic 확인
//    if (!GetCapsuleComponent() || !GetCapsuleComponent()->BodyInstance)
//    {
//        return;
//    }
//    PxRigidDynamic* RigidActor = GetCapsuleComponent()
//        ->BodyInstance
//        ->BIGameObject
//        ->DynamicRigidBody;
//    if (!RigidActor)
//    {
//        return;
//    }
//
//    // 2) PhysX 속도(PxVec3) --> FVector로 변환
//    PxVec3 PxVelocity = RigidActor->getLinearVelocity();
//    FVector Velocity = FVector(PxVelocity.x, PxVelocity.y, PxVelocity.z);
//
//    // 3) XY 평면 성분만 취하고, 너무 작은 속도는 무시
//    FVector FlatVel = Velocity;
//    FlatVel.Z = 0.f;
//    // XY 속도가 거의 0이면 더 이상 회전 처리 불필요
//    if (FlatVel.IsNearlyZero(1e-3f))
//    {
//        return;
//    }
//
//    // 4) 목표 Yaw 계산 (FlatVel의 방향)
//    float TargetYaw = FlatVel.Rotation().Yaw;
//
//    // 5) 현재 Actor의 Yaw 가져오기
//    FRotator CurrentRotator = GetActorRotation();
//    float CurrentYaw = CurrentRotator.Yaw;
//
//    // 6) 두 Yaw 사이의 최소 각도 차이(Shortest Angle) 구하기
//    float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw);
//    const float AngleThreshold = 1.0f; // 1도 미만 변화는 무시
//    if (FMath::Abs(DeltaYaw) < AngleThreshold)
//    {
//        return;
//    }
//
//    // 7) 부드럽게 보간된 새 Yaw 계산 (속도 계수는 10.0f, 필요 시 변경)
//    float NewYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, 10.0f);
//
//    // 8) Actor 레벨에서 Yaw만 업데이트 (Pitch/Roll 은 0)
//    FRotator NewRotation(0.f, NewYaw, 0.f);
//    SetActorRotation(NewRotation);
//
//    // 9) PhysX Body에도 최소한의 회전만 반영
//    //    위치는 물리가 관리하므로 그대로 두고, 회전만 덮어씌웁니다.
//    {
//        PxTransform PxCurrentTM = RigidActor->getGlobalPose();
//        FQuat    NewQuat = NewRotation.Quaternion();
//        PxQuat   PxNewQuat(
//            NewQuat.X,
//            NewQuat.Y,
//            NewQuat.Z,
//            NewQuat.W
//        );
//        PxCurrentTM.q = PxNewQuat;
//        RigidActor->setGlobalPose(PxCurrentTM, false);
//    }
//}

//void APlayer::UpdateFacingRotation(float DeltaTime)
//{
//    if (!GetCapsuleComponent() || !GetCapsuleComponent()->BodyInstance)
//    {
//        return;
//    }
//    PxVec3 PxVelocity = GetCapsuleComponent()->BodyInstance->BIGameObject->DynamicRigidBody->getLinearVelocity();
//    FVector Velocity = FVector(PxVelocity.x, PxVelocity.y, PxVelocity.z);
//    FRotator CurrentRotation = GetActorRotation();
//    // 목표 방향의 Yaw만 추출
//    float TargetYaw = Velocity.Rotation().Yaw;
//
//    // 부드럽게 보간
//    float NewYaw = FMath::FInterpTo(CurrentRotation.Yaw, TargetYaw, DeltaTime, 10.f);
//    UE_LOG(ELogLevel::Display, "Target Yaw : %f", TargetYaw);
//    UE_LOG(ELogLevel::Display,"New Yaw : %f", NewYaw);
//    FRotator NewRotation = FRotator(0.f, NewYaw, 0.f);
//
//    // Pitch, Roll은 고정하고 Yaw만 변경
//
//    //FQuat NewQuat = NewRotation.Quaternion();
//
//    //// RigidBody 얻기 (BodyInstance 또는 직접 보유한 PxRigidDynamic*)
//    //if (SkeletalMeshComponent->BodyInstance)
//    //{
//    //    PxRigidDynamic* RigidBody = SkeletalMeshComponent->BodyInstance->GetPxRigidDynamic_AssumesLocked();
//    //    if (RigidBody)
//    //    {
//    //        PxTransform CurrentPose = RigidBody->getGlobalPose();
//    //        CurrentPose.q = U2PQuat(NewQuat); // Unreal → PhysX 회전만 변경
//    //        RigidBodydf->setGlobalPose(CurrentPose, false); // 위치 유지, 회전만 적용
//    //    }
//    //}
//
//
//    if (PxRigidDynamic* RigidActor = GetCapsuleComponent()->BodyInstance->BIGameObject->DynamicRigidBody)
//    {
//        PxTransform CurrentTransform = RigidActor->getGlobalPose();
//        FQuat NewQuat = NewRotation.Quaternion();
//
//        // 절대 회전 덮어쓰기 (누적 X)
//        CurrentTransform.q = PxQuat(NewQuat.X, NewQuat.Y, NewQuat.Z, NewQuat.W);
//        RigidActor->setGlobalPose(CurrentTransform, false);
//    }
//}

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
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaTime * PitchSpeed, -30.0f, 15.0f);
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

void APlayer::SetControllerVibration(float LeftMotor, float RightMotor) const
{
    // XINPUT_VIBRATION 구조체 설정
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    
    // 진동 강도 설정 (0.0f ~ 1.0f를 0 ~ 65535로 변환)
    vibration.wLeftMotorSpeed = (WORD)(LeftMotor * 65535.0f);   // 저주파 모터 (왼쪽)
    vibration.wRightMotorSpeed = (WORD)(RightMotor * 65535.0f); // 고주파 모터 (오른쪽)
    
    // 진동 설정 적용
    XInputSetState(PlayerIndex, &vibration);
}

void APlayer::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APlayer, sol::bases<AActor, APawn, ACharacter>(),
    "Acceleration", &APlayer::Acceleration,
    "MaxSpeed", &APlayer::MaxSpeed,
    "RawSpeed", &APlayer::RawSpeed,
    "PitchSpeed", &APlayer::PitchSpeed,
    "ChangeViewTarget", &APlayer::ChangeTargetViewPlayer,
    "SetControllerVibration", &APlayer::SetControllerVibration,
    "LinearSpeed", sol::property(&APlayer::GetLinearSpeed, &APlayer::SetLinearSpeed)
    )
}

void APlayer::OnDamaged(FVector KnockBackDir)
{
    Super::OnDamaged(KnockBackDir);
    UWorld* World = GetWorld();
    World->GetPlayerController(PlayerIndex)->PlayerCameraManager->StartCameraShake(UDamageCameraShake::StaticClass());
    CollisionComponent->BodyInstance->AddForce(KnockBackDir * KnockBackPower * KnockBackExp, true);
}


void APlayer::StartGame()
{
    if (UWorld* World = GetWorld())
    {
        World->GetGameMode()->StartMatch();
    }

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

void APlayer::OnDead() const
{
    LuaScriptComponent->ActivateFunction("OnDead");
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
