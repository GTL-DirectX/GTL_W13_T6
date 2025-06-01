#include "Weapon.h"

#include "UObject/ObjectFactory.h"

#include "WeaponComponent.h"
#include "Components/SphereComponent.h"

#include "GameFramework/Player.h"

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void AWeapon::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    
    if (!WeaponMesh)
    {
        WeaponMesh = FObjectFactory::ConstructObject<UWeaponComponent>(this);
    }

    CollisionSphere = FObjectFactory::ConstructObject<USphereComponent>(this);
    if (CollisionSphere)
    {
        CollisionSphere->OnComponentBeginOverlap.AddUObject(this, &AWeapon::OnComponentBeginOverlap);
    }
}

UObject* AWeapon::Duplicate(UObject* InOuter)
{
    AWeapon* NewWeapon = FObjectFactory::ConstructObject<AWeapon>(InOuter, GetName());

    if (NewWeapon)
    {
        NewWeapon->WeaponMesh = WeaponMesh;
        NewWeapon->CollisionSphere = Cast<USphereComponent>(CollisionSphere->Duplicate(InOuter));
    }

    return NewWeapon;
}

void AWeapon::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    if (!OtherActor || !OtherComp || Cast<APlayer>(OtherActor))
    {
        return;
    }

    // 캐릭터랑 충돌 되면 캐릭터 무기 컴포넌트에 장착.

    // 캐릭터 무기 장착 로직. Equip 되는 경우이므로 다른 함수로 분리 필요.
    // Component 자체를 옮겨줌으로써 Melee, Ranged 모두 사용 가능하도록 함.
    // APlayer* Character = Cast<APlayer>(OtherActor);
    // Character->EquipWeapon(WeaponMesh);
    // WeaponMesh = nullptr;
    // Destroy();
}
