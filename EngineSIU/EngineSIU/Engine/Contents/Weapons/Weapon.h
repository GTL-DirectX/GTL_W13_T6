#pragma once

#include "GameFramework/Actor.h"

class UWeaponComponent;
class USphereComponent;
class UPrimitiveComponent;

class AWeapon : public AActor
{
    DECLARE_CLASS(AWeapon, AActor)

public:
    AWeapon() = default;

public:
    UWeaponComponent* GetWeaponMesh() const { return WeaponMesh; }

    virtual void BeginPlay() override;
    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

private:
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor*  OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

private:
    UPROPERTY(EditAnywhere, UWeaponComponent*, WeaponMesh, = nullptr)

    USphereComponent* CollisionSphere = nullptr;


};

