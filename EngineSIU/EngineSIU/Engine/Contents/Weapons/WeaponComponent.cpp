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
    if (OwnerCharacter && OwnerCharacter->GetState() < static_cast<int>(EPlayerState::Attacking))
    {
        bIsAttacking = true;
        OwnerCharacter->SetState(static_cast<int>(EPlayerState::Attacking));
    }
}

void UWeaponComponent::FinishAttack()
{
    if (OwnerCharacter && OwnerCharacter->GetState() < static_cast<int>(EPlayerState::Stun))
    {
        bIsAttacking = false;
        OwnerCharacter->SetState(static_cast<int>(EPlayerState::Idle));
    }
}
