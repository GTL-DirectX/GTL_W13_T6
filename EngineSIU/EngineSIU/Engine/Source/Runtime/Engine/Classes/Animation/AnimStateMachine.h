#pragma once
#include<UObject/Object.h>
#include "UObject/ObjectMacros.h"

#include "sol/sol.hpp"

class USkeletalMeshComponent;
class ULuaScriptAnimInstance;
class APawn;

class UAnimStateMachine : public UObject
{
    DECLARE_CLASS(UAnimStateMachine, UObject)

public:
    UAnimStateMachine();
    virtual ~UAnimStateMachine() override = default;

    virtual void Initialize(USkeletalMeshComponent* InOwner, ULuaScriptAnimInstance* InAnimInstance);

    void ProcessState();
    
    void InitLuaStateMachine();
    
    FString GetLuaScriptName() const { return LuaScriptName; }

    USkeletalMeshComponent* OwningComponent;
    ULuaScriptAnimInstance* OwningAnimInstance;
    
public:
    UPROPERTY(EditAnywhere, FString, LuaScriptName, = TEXT(""));

private:
    sol::table LuaTable = {};

};
