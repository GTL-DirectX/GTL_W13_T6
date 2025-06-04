#pragma once
#include "AnimNotify.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAnimNotify, USkeletalMeshComponent*, UAnimSequenceBase*);


class UAnimDelegateNotify : public UAnimNotify
{
    DECLARE_CLASS(UAnimDelegateNotify, UAnimNotify)
public:
    UAnimDelegateNotify() = default;

    /**
     * Notify가 실행될 때 호출되는 함수 (엔진이 자동으로 호출)
     * 여기서 델리게이트를 브로드캐스트한다.
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:
    /** 이 Notify에 바인딩할 수 있는 델리게이트 */
    FOnAnimNotify OnNotifyTriggered;
};

