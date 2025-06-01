#pragma once
#include "Pawn.h"

class USkeletalMeshComponent;

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)
public:
    ACharacter() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override { Super::Tick(DeltaTime); }

    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override { }

    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
    
protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
};
