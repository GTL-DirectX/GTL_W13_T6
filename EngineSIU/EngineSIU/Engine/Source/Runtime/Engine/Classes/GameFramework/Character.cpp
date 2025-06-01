#include "Character.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"

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

    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    }

    if (!CapsuleComponent)
    {
        CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
    }

}

void ACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>())
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
