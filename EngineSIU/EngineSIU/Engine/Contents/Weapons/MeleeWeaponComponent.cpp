#include "MeleeWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Player.h"

void MeleeWeaponComponent::Attack()
{
    if (!OwnerCharacter)
    {
        return;
    }

    // OwnerActor에 Animation 넣어서 재생해주기.
    if (USkeletalMeshComponent* SkelComp = OwnerCharacter->GetSkeletalMeshComponent())
    {
        SkelComp->PlayAnimation(AttackAnimation,  1.0f, false);
    }
}
