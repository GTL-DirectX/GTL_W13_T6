#include "Monster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"

class ULuaScriptAnimInstance;

UObject* AMonster::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}

void AMonster::PostSpawnInitialize()
{
    TargetPos = FDistributionVector(FVector(-140.0f, -140.0f, 0.0f), FVector(140.0f, 140.0f, 0.f));
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);
    
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Bowser/Bowser_Hit")));
    /*SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    SkeletalMeshComponent->SetAnimClass(UClass::FindClass(FName("ULuaScriptAnimInstance")));*/
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);
}
void AMonster::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AMonster,
        sol::bases<AActor, ACharacter, APawn>(),
        "GetTargetPosition", &ThisClass::GetTargetPosition,
        "IsFalling", &ThisClass::IsFalling
    )
    /*DEFINE_LUA_TYPE_NO_PARENT(AMonster,
        "GetTargetPosition", &ThisClass::GetTargetPosition
    )*/
}

bool AMonster::BindSelfLuaProperties()
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
    LuaTable["Name"] = *GetName();

    return true;
}

FVector AMonster::GetTargetPosition()
{
    return TargetPos.GetValue();
}


void AMonster::BeginPlay()
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
void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool AMonster::IsFalling() const
{
    return true;
}

