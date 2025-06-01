#pragma once
#include "Character.h"
#include "Distribution/DistributionVector.h"

class USkeletalMeshComponent;

class AMonster : public ACharacter
{
    DECLARE_CLASS(AMonster, APawn)
public:
    AMonster() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    FVector GetTargetPosition();

protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/Monster.lua")
    UPROPERTY(EditAnywhere, FString, StateMachineFileName, = "LuaScripts/Animations/MonsterStateMachine.lua");

    FDistributionVector TargetPos;
};
