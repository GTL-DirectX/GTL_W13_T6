#include "Monster.h"
#include "Components/SkeletalMeshComponent.h"

UObject* AMonster::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->SkeletalMeshComponent = NewActor->GetComponentByClass<USkeletalMeshComponent>();

    return NewActor;
}

void AMonster::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    SkeletalMeshComponent->SetSkeletalMeshAsset(
    UAssetManager::Get().GetSkeletalMesh(FName("Contents/Bowser/Bowser_Hit"))
    );


}
void AMonster::BeginPlay()
{
    Super::BeginPlay();

}
void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

