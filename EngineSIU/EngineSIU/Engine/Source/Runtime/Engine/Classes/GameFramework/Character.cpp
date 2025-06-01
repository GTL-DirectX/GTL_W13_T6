#include "Character.h"

#include "Components/SkeletalMeshComponent.h"

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}

void ACharacter::PostSpawnInitialize()
{
    APawn::PostSpawnInitialize();

    RootComponent = AddComponent<USceneComponent>();

    if (!SkeletalMeshComponent)
    {
        SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    }

}
