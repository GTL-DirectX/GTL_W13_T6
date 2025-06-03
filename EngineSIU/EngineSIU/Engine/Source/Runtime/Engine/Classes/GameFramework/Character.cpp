#include "Character.h"

#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"

#include "Engine/EditorEngine.h"

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    if (NewActor)
    {

        NewActor->CapsuleComponent = GetComponentByClass<UCapsuleComponent>();
        //NewActor->CapsuleComponent = Cast<UCapsuleComponent>(CapsuleComponent->Duplicate(InOuter));
        NewActor->SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>();
        //NewActor->SkeletalMeshComponent = Cast<USkeletalMeshComponent>(SkeletalMeshComponent->Duplicate(InOuter));
     }

    return NewActor;
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    if (!CapsuleComponent)
    {
        CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
        CapsuleComponent->SetupAttachment(RootComponent);
        //CapsuleComponent->AddScale(FVector(5.0f, 5.0f, 5.0f));
        CapsuleComponent->AddLocation({ 0.0f, 0.0f, 0.0f });
        CapsuleComponent->bSimulate = true;
        CapsuleComponent->bApplyGravity = true;
        CapsuleComponent->RigidBodyType = ERigidBodyType::DYNAMIC;

        AggregateGeomAttributes CapsuleGeomAttributes;
        CapsuleGeomAttributes.GeomType = EGeomType::ECapsule;
        CapsuleGeomAttributes.Offset = FVector::ZeroVector;
        CapsuleGeomAttributes.Extent = FVector(1.0f, 1.0f, 1.0f); // Half Extent
        CapsuleGeomAttributes.Rotation = FRotator(90.0f, 0.0f, 00.0f);
        CapsuleComponent->GeomAttributes.Add(CapsuleGeomAttributes);
    }

    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
        SkeletalMeshComponent->SetupAttachment(RootComponent);
        SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Human/Human")));
        SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        SkeletalMeshComponent->SetAnimClass(UClass::FindClass(FName("ULuaScriptAnimInstance")));
    }
}

void ACharacter::BeginPlay()
{
    Super::BeginPlay();

    if (CapsuleComponent)
    {
        CapsuleComponent->CreatePhysXGameObject();
        CapsuleComponent->BodyInstance->BIGameObject->DynamicRigidBody->
        setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X
            | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y); // X, Y축 회전 잠금
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
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>(),
        "State", sol::property(&APlayer::GetState, &APlayer::SetState),
        "IsGrounded", &ACharacter::CheckGrounded
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
    float GroundCheckDistance = CapsuleComponent->GetHalfHeight() + 1.0f;
    FVector Start = CapsuleComponent->GetComponentLocation();
    FVector End = Start - FVector(0.0f, 0.0f, GroundCheckDistance);

    PxRaycastBuffer Hit;
    PxQueryFilterData Filter;
    Filter.flags = PxQueryFlag::eSTATIC;

    bool bHit = GEngine->PhysicsManager->GetScene(GetWorld())->raycast(PxVec3(Start.X, Start.Y, Start.Z), PxVec3(End.X, End.Y, End.Z) - PxVec3(Start.X, Start.Y, Start.Z), GroundCheckDistance, Hit, PxHitFlag::eDEFAULT, Filter);

    return bHit && Hit.block.distance <= GroundCheckDistance;
}

