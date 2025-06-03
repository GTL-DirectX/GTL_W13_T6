#include "MeleeWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Player.h"

#include "GameFramework/Monster.h"


void MeleeWeaponComponent::Attack()
{
    if (!OwnerCharacter)
    {
        return;
    }
}

void MeleeWeaponComponent::InitializeComponent()
{
    Super::InitializeComponent();

    OnComponentBeginOverlap.AddUObject(this, &MeleeWeaponComponent::ComponentBeginOverlap);

}

void MeleeWeaponComponent::ComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    if (bIsAttacking)
    {
        if (AMonster* Monster = Cast<AMonster>(OtherActor))
        {
            
        }
        else if (APlayer* OtherPlayer = Cast<APlayer>(OtherActor))
        {

        }
    }
}
