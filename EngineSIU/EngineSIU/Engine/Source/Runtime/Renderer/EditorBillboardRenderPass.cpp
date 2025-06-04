
#include "EditorBillboardRenderPass.h"

#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "UObject/UObjectIterator.h"
#include "Components/BillboardComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

FEditorBillboardRenderPass::FEditorBillboardRenderPass()
{
    ResourceType = EResourceType::ERT_EditorOverlay;
}

void FEditorBillboardRenderPass::PrepareRenderArr()
{
    BillboardComps.Empty();
    for (const auto Component : TObjectRange<UBillboardComponent>())
    {
        if (Component->GetWorld() == GEngine->ActiveWorld && Component->bIsEditorBillboard)
        {
            BillboardComps.Add(Component);
        }
    }

    const FVector LocCam = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraLocation();
    BillboardComps.Sort(
        [LocCam](const UBillboardComponent* A, const UBillboardComponent* B)
        {
            const FVector LocA = A->GetComponentLocation();
            const FVector LocB = B->GetComponentLocation();

            const float DistA = (LocCam - LocA).SquaredLength();
            const float DistB = (LocCam - LocB).SquaredLength();

            return DistA > DistB;
        }
    );
}
