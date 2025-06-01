#include "WeaponComponent.h"

#include "GameFramework/Character.h"

void UWeaponComponent::InitializeComponent()
{
    Super::InitializeComponent();
    if (GetOwner())
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }
    
}

void UWeaponComponent::Attack()
{
    
}
