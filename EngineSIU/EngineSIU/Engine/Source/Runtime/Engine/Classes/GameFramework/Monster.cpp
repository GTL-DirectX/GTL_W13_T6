#include "Monster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "SoundManager.h"
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
    TargetDistributionVector = FDistributionVector(FVector(-140.0f, -140.0f, 0.0f), FVector(140.0f, 140.0f, 0.f));
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
        "GetTargetPosition", &ThisClass::UpdateTargetPosition,
        "UpdateTargetPosition", &ThisClass::UpdateTargetPosition,
        "FollowTimer", sol::property(&ThisClass::GetFollowTimer, &ThisClass::SetFollowTimer),
        "TargetPosition", sol::property(&ThisClass::GetTargetPosition, &ThisClass::SetTargetPosition),

        "IsHit", sol::property(&ThisClass::IsHit, &ThisClass::SetHit),
        "IsDead", sol::property(&ThisClass::IsDead, &ThisClass::SetDead),
        "IsChasing", sol::property(&ThisClass::GetIsChasing, &ThisClass::SetIsChasing),
        "IsFalling", sol::property(&ThisClass::IsFalling, &ThisClass::SetFalling),
        "IsFallingToDeath", sol::property(&ThisClass::IsFallingToDeath, &ThisClass::SetFallingToDeath)
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

void AMonster::UpdateTargetPosition()
{
    TargetPos = TargetDistributionVector.GetValue();
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
    FSoundManager::GetInstance().PlaySound("SpawnKoopa");


}
void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool AMonster::IsFalling() const
{
    return GetActorLocation().Z > 0.2f;
}

