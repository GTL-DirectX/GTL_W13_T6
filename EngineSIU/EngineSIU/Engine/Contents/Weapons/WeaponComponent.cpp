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

UObject* UWeaponComponent::Duplicate(UObject* InOuter)
{
    UWeaponComponent* NewComponent = Cast<UWeaponComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->AttackDamage = AttackDamage;
    }
    return NewComponent;
}

void UWeaponComponent::Attack()
{
    
}
