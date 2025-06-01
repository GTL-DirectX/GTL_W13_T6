#pragma once
#include "Character.h"

class USkeletalMeshComponent;
class UCameraComponent;

class ASequencerPlayer : public ACharacter
{
    DECLARE_CLASS(ASequencerPlayer, ACharacter)

public:
    ASequencerPlayer();
    virtual ~ASequencerPlayer() override = default;

    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    FName Socket = "jx_c_camera";
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;

private:
    UCameraComponent* CameraComponent = nullptr;
};
