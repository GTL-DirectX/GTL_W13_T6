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
    DECLARE_CLASS(UWeaponComponent, USceneComponent)

public:
    UWeaponComponent() = default;

    virtual void InitializeComponent() override;

public:
    virtual void Attack();

protected:
    ACharacter* OwnerCharacter = nullptr;

private:
    UStaticMesh* WeaponMesh = nullptr;

public:
    float GetAttackDamage() const { return AttackDamage; }

protected:
    float AttackDamage = 10.0f;



};

