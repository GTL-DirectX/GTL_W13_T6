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

    template<typename T>
    void BindTargetActor(T* TargetActor);
    
private:
    UPROPERTY(EditAnywhere, FString, LuaScriptName, = TEXT(""));
    sol::table LuaTable = {};

};

template<typename T>
inline void UAnimStateMachine::BindTargetActor(T* TargetActor)
{
    if (!LuaTable.valid() || !TargetActor)
    {
        return;
    }

    if constexpr (std::is_base_of<APawn, T>::value)
    {
        LuaTable["OwnerCharacter"] = TargetActor;
    }
}
