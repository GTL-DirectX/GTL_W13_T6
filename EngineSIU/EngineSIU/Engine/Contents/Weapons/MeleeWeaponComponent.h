#pragma once

#include "WeaponComponent.h"

class UAnimSequence;
class USphereComponent;

class UMeleeWeaponComponent : public UWeaponComponent
{
    DECLARE_CLASS(UMeleeWeaponComponent, UWeaponComponent)

public:
    UMeleeWeaponComponent() = default;

    virtual void Attack() override;
    virtual void InitializeComponent() override;

protected:
    virtual void ComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

protected:
    UAnimSequence* AttackAnimation = nullptr; // 공격 애니메이션

    float AttackSpeed = 1.0f; // 공격 속도
    float AttackRange = 100.0f; // 공격 범위
    float AttackRadius = 50.0f; // 공격 반경

    USphereComponent* AttackCollision; // 공격 충돌 컴포넌트

};

