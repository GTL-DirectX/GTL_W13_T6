#include "Monster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"

UObject* AMonster::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->SkeletalMeshComponent = NewActor->GetComponentByClass<USkeletalMeshComponent>();

    return NewActor;
}

void AMonster::PostSpawnInitialize()
{
    TargetPos = FDistributionVector(FVector(-140.0f, -140.0f, 0.0f), FVector(140.0f, 140.0f, 0.f));
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);
    
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Bowser/Bowser_Hit")));
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);
}
void AMonster::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AMonster,
        sol::bases<AActor, ACharacter, APawn>(),
        "GetTargetPosition", &ThisClass::GetTargetPosition/*,
        "IsFalling", &ThisClass::IsFalling*/
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

}
void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

