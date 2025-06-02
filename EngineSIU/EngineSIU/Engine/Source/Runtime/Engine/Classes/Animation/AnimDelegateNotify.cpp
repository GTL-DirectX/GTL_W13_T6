#include "AnimDelegateNotify.h"

void UAnimDelegateNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    OnNotifyTriggered.Broadcast(MeshComp, Animation);
}
