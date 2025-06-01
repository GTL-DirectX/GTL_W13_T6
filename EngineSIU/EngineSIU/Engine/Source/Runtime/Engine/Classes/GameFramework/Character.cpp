#include "Character.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    if (NewActor)
    {
        NewActor->CapsuleComponent = Cast<UCapsuleComponent>(CapsuleComponent->Duplicate(InOuter));
        NewActor->SkeletalMeshComponent = Cast<USkeletalMeshComponent>(SkeletalMeshComponent->Duplicate(InOuter));
    }

    return NewActor;
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    RootComponent = AddComponent<USceneComponent>();


    if (!CapsuleComponent)
    {
        CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
    }

    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
        SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Human/Human")));
        SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        SkeletalMeshComponent->SetAnimClass(UClass::FindClass(FName("ULuaScriptAnimInstance")));
    }
}

void ACharacter::BeginPlay()
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

void ACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    Test = "Test";
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>(),
        "Test", &ACharacter::Test
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
