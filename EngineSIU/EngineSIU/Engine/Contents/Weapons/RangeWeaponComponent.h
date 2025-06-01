#pragma once

#include "WeaponComponent.h"

class URangeWeaponComponent : public UWeaponComponent
{
    DECLARE_CLASS(URangeWeaponComponent, UWeaponComponent)

public:
    URangeWeaponComponent() = default;

    virtual void Attack() override;

private:
    

protected:
    float ProjectileSpeed = 1000.0f; // 발사체 속도
    float MaxRange = 2000.0f; // 최대 사거리
    float ReloadTime = 2.0f; // 재장전 시간

};

