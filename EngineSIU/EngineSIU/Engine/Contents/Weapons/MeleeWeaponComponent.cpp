#include "MeleeWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Player.h"

#include "GameFramework/Monster.h"


void UMeleeWeaponComponent::Attack()
{
    Super::Attack();
    if (!OwnerCharacter)
    {
        return;
    }
}

void UMeleeWeaponComponent::InitializeComponent()
{
    Super::InitializeComponent();

    OnComponentBeginOverlap.AddUObject(this, &UMeleeWeaponComponent::ComponentBeginOverlap);

    if (!AttackCollision && GetOwner())
    {
        AttackCollision = GetOwner()->AddComponent<USphereComponent>("AttackCollision");
        AttackCollision->SetupAttachment(this);
        AttackCollision->AddLocation(FVector(0.0f, -0.1f, -0.3f));
        AttackCollision->SetRadius(0.7f);
        AttackCollision->OnComponentBeginOverlap.AddUObject(this, &UMeleeWeaponComponent::ComponentBeginOverlap);
    }

}

void UMeleeWeaponComponent::ComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    UE_LOG(ELogLevel::Warning, TEXT("MeleeWeapon BeginOverlap"));
    if (bIsAttacking)
    {
        if (AMonster* Monster = Cast<AMonster>(OtherActor))
        {
            FVector DamageDir = OtherActor->GetActorLocation() - GetComponentLocation();
            Monster->OnDamaged(DamageDir);
        }
        else if (APlayer* OtherPlayer = Cast<APlayer>(OtherActor))
        {
            FVector DamageDir = OtherActor->GetActorLocation() - GetComponentLocation();
            OtherPlayer->OnDamaged(DamageDir);
        }
    }
}
