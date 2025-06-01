#pragma once
#include "Character.h"

class USkeletalMeshComponent;

class AMonster : public ACharacter
{
    DECLARE_CLASS(AMonster, APawn)
public:
    AMonster() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void PostSpawnInitialize() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
