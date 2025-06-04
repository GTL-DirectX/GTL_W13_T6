
#pragma once
#include "Camera/CameraShakeBase.h"

class UDamageCameraShake : public UCameraShakeBase
{
    DECLARE_CLASS(UDamageCameraShake, UCameraShakeBase)

public:
    UDamageCameraShake();
    virtual ~UDamageCameraShake() override = default;

    
};

class UHitCameraShake : public UCameraShakeBase
{
    DECLARE_CLASS(UHitCameraShake, UCameraShakeBase)

public:
    UHitCameraShake();
    virtual ~UHitCameraShake() override = default;


};
