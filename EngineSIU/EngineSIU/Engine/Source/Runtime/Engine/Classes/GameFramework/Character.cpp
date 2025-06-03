#include "Character.h"

#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Classes/Particles/ParticleSystemComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"

#include "Engine/EditorEngine.h"

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    if (NewActor)
    {

        NewActor->CollisionComponent = GetComponentByClass<UBoxComponent>();
        //NewActor->CapsuleComponent = Cast<UCapsuleComponent>(CapsuleComponent->Duplicate(InOuter));
        NewActor->SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>();
        //NewActor->SkeletalMeshComponent = Cast<USkeletalMeshComponent>(SkeletalMeshComponent->Duplicate(InOuter));
       // NewActor->ParticleSystemComponent = GetComponentByClass<UParticleSystemComponent>();
  
    }

    return NewActor;
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    if (!CollisionComponent)
    {
        CollisionComponent = AddComponent<UBoxComponent>("CollisionComponent");
        CollisionComponent->SetupAttachment(RootComponent);
        CollisionComponent->AddLocation({ 0.0f, 0.0f, 0.0f });
        CollisionComponent->bSimulate = true;
        CollisionComponent->bApplyGravity = true;
        CollisionComponent->RigidBodyType = ERigidBodyType::DYNAMIC;
        CollisionComponent->SetBoxExtent(FVector(3.0f, 3.0f, 9.0f)); // Half Extent

        AggregateGeomAttributes CapsuleGeomAttributes;
        CapsuleGeomAttributes.GeomType = EGeomType::ECapsule;
        CapsuleGeomAttributes.Offset = FVector::ZeroVector;
        CapsuleGeomAttributes.Extent = FVector(1.0f, 1.0f, 1.0f); // Half Extent
        CapsuleGeomAttributes.Rotation = FRotator(90.0f, 0.0f, 00.0f);
        CollisionComponent->GeomAttributes.Add(CapsuleGeomAttributes);
    }

    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
        SkeletalMeshComponent->SetupAttachment(RootComponent);
        SkeletalMeshComponent->AddLocation({ 0.0f, 0.0f, -10.0f });
        SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Human/Human")));
        SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        SkeletalMeshComponent->SetAnimClass(UClass::FindClass(FName("ULuaScriptAnimInstance")));
    }
}

void ACharacter::BeginPlay()
{
    Super::BeginPlay();

    if (CollisionComponent)
    {
        CollisionComponent->CreatePhysXGameObject();
        CollisionComponent->BodyInstance->BIGameObject->DynamicRigidBody->
        setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X
            | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z); // X, Y축 회전 잠금
    }

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

void ACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveSpeed = Velocity.Length();
    CollisionComponent->BodyInstance->AddForce(Velocity);
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>(),
        "State", sol::property(&ACharacter::GetState, &ACharacter::SetState),
        "MoveSpeed", &ACharacter::MoveSpeed,
        "Velocity", &ACharacter::Velocity,
        "StunGauge", &ACharacter::StunGauge,
        "MaxStunGauge", &ACharacter::MaxStunGauge,
        "KnockBackPower", &ACharacter::KnockBackPower,
        "IsGrounded", &ACharacter::CheckGrounded// CheckGrounded는 bool 반환
        )
}

bool ACharacter::BindSelfLuaProperties()
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

void ACharacter::SetState(int State)
{
    PlayerState = static_cast<EPlayerState>(State);
}

int ACharacter::GetState()
{
    return static_cast<int>(PlayerState);
}

void ACharacter::OnDamaged(FVector KnockBackDir)
{
    LuaScriptComponent->ActivateFunction("OnDamaged", KnockBackDir);
}

void ACharacter::Jump()
{
    if (!bIsGrounded || PlayerState == EPlayerState::Jumping)
    {
        return;
    }

    FVector JumpImpulse = FVector(0.0f, 0.0f, 300.0f);
    //CapsuleComponent->BodyInstance->AddImpulse();


    bIsGrounded = false;
    PlayerState = EPlayerState::Jumping;
}

bool ACharacter::CheckGrounded()
{
    float GroundCheckDistance = CollisionComponent->GetBoxExtent().Z + 1.0f;
    FVector Start = CollisionComponent->GetComponentLocation();
    FVector End = Start - FVector(0.0f, 0.0f, GroundCheckDistance);

    PxRaycastBuffer Hit;
    PxQueryFilterData Filter;
    Filter.flags = PxQueryFlag::eSTATIC;

    bool bHit = GEngine->PhysicsManager->GetScene(GetWorld())->raycast(PxVec3(Start.X, Start.Y, Start.Z), PxVec3(End.X, End.Y, End.Z) - PxVec3(Start.X, Start.Y, Start.Z), GroundCheckDistance, Hit, PxHitFlag::eDEFAULT, Filter);

    return bHit && Hit.block.distance <= GroundCheckDistance;
}

