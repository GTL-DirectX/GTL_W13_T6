#pragma once

#include "Components/StaticMeshComponent.h"

enum class EWeaponType
{
    Melee,
    Ranged,
    Max
};

class ACharacter;

class UWeaponComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UWeaponComponent, UStaticMeshComponent)

public:
    UWeaponComponent() = default;

    virtual void InitializeComponent() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

public:
    virtual void Attack();
    virtual void FinishAttack();

protected:
    ACharacter* OwnerCharacter = nullptr;

public:
    float GetAttackDamage() const { return AttackDamage; }

protected:
    float AttackDamage = 10.0f;
    bool bIsAttacking = false;


};

