#include "LuaScriptAnimInstance.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimationRuntime.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/FrameTime.h"
#include "Animation/AnimStateMachine.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimData/AnimDataModel.h"

ULuaScriptAnimInstance::ULuaScriptAnimInstance()
    : PrevAnim(nullptr)
    , CurrAnim(nullptr)
    , ElapsedTime(0.f)
    , PreElapsedTime(0.f)
    , PlayRate(1.f)
    , bLooping(true)
    , bPlaying(true)
    , bReverse(false)
    , LoopStartFrame(0)
    , LoopEndFrame(0)
    , CurrentKey(0)
    , BlendAlpha(0.f)
    , BlendStartTime(0.f)
    , BlendDuration(0.2f)
    , bIsBlending(false)
{
}

void ULuaScriptAnimInstance::InitializeAnimation()
{
    UAnimInstance::InitializeAnimation();
    
    StateMachine = FObjectFactory::ConstructObject<UAnimStateMachine>(this);
    StateMachine->Initialize(Cast<USkeletalMeshComponent>(GetOuter()), this);
}

void ULuaScriptAnimInstance::NativeInitializeAnimation()
{
}

UObject* ULuaScriptAnimInstance::Duplicate(UObject* InOuter)
{
    ULuaScriptAnimInstance* NewInstance = Cast<ULuaScriptAnimInstance>(Super::Duplicate(InOuter));
    if (NewInstance)
    {
        if (StateMachine)
        {
            NewInstance->StateMachine = Cast<UAnimStateMachine>(StateMachine->Duplicate(InOuter)); 
            NewInstance->StateMachine->Initialize(Cast<USkeletalMeshComponent>(InOuter), NewInstance);
        }
    }
    return NewInstance;
}
void ULuaScriptAnimInstance::NativeUpdateAnimation(float DeltaSeconds, FPoseContext& OutPose)
{
    UAnimInstance::NativeUpdateAnimation(DeltaSeconds, OutPose);
    StateMachine->ProcessState(/*DeltaSeconds*/);

#pragma region MyAnim
    USkeletalMeshComponent* SkeletalMeshComp = GetSkelMeshComponent();

    if (!PrevAnim || !CurrAnim || !SkeletalMeshComp->GetSkeletalMeshAsset() || !SkeletalMeshComp->GetSkeletalMeshAsset()->GetSkeleton() || !bPlaying)
    {
        return;
    }

    float PrevPlayRate = PrevAnim->RateScale;
    PreviousTime = ElapsedTime;
    PreElapsedTime += DeltaSeconds * PrevPlayRate;
    ElapsedTime += DeltaSeconds * PlayRate;

    const float AnimDuration = CurrAnim->GetDuration();
    auto tempPreviousTime = FMath::Clamp(PreviousTime, 0.0f, AnimDuration);
    auto tempElapsedTime = FMath::Clamp(ElapsedTime, 0.0f, AnimDuration);

    CurrAnim->EvaluateAnimNotifies(CurrAnim->Notifies, tempElapsedTime, tempPreviousTime, DeltaSeconds, SkeletalMeshComp, CurrAnim, bLooping);

    //CurrAnim->EvaluateAnimNotifies(CurrAnim->Notifies, ElapsedTime, PreviousTime, DeltaSeconds, SkeletalMeshComp, CurrAnim, bLooping);

    if (CurrAnim && !bLooping)
    {
        const float AnimDuration = CurrAnim->GetPlayLength();

        ElapsedTime = FMath::Clamp(
            ElapsedTime,
            0.0f,
            FMath::FloorToFloat(AnimDuration * 10000) / 10000
        );
    }
    if (bIsBlending && PreElapsedTime <= PrevAnim->GetDuration())
    {
        // ✅ BlendElapsed는 단순히 PreElapsedTime을 사용 (BlendStartTime은 항상 0)
        float BlendElapsed = PreElapsedTime;
        BlendAlpha = FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f);

        if (BlendAlpha >= 1.f)
        {
            bIsBlending = false;
            PrevAnim = CurrAnim;
        }
    }
    else
    {
        BlendAlpha = 1.f;
    }

    /*if (bIsBlending && PreElapsedTime <= PrevAnim->GetDuration())
    {
        float BlendElapsed = ElapsedTime - BlendStartTime;
        BlendAlpha = FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f);

        if (BlendAlpha >= 1.f)
        {
            bIsBlending = false;
            PrevAnim = CurrAnim;
        }
    }
    else
    {
        BlendAlpha = 1.f;
    }*/

    // TODO: FPoseContext의 BoneContainer로 바꾸기
    const FReferenceSkeleton& RefSkeleton = this->GetCurrentSkeleton()->GetReferenceSkeleton();

    if (PrevAnim->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum() != RefSkeleton.RawRefBoneInfo.Num() || CurrAnim->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum() != RefSkeleton.RawRefBoneInfo.Num())
    {
        return;
    }

    FPoseContext PrevPose(this);
    FPoseContext CurrPose(this);

    PrevPose.Pose.InitBones(RefSkeleton.RawRefBoneInfo.Num());
    CurrPose.Pose.InitBones(RefSkeleton.RawRefBoneInfo.Num());
    for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.RawRefBoneInfo.Num(); ++BoneIdx)
    {
        PrevPose.Pose[BoneIdx] = RefSkeleton.RawRefBonePose[BoneIdx];
        CurrPose.Pose[BoneIdx] = RefSkeleton.RawRefBonePose[BoneIdx];
    }

    FAnimExtractContext ExtractA(PreElapsedTime, bLooping); // TODO : bPrevLooping
    FAnimExtractContext ExtractB(ElapsedTime, bLooping);

    PrevAnim->GetAnimationPose(PrevPose, ExtractA);
    CurrAnim->GetAnimationPose(CurrPose, ExtractB);

    FAnimationRuntime::BlendTwoPosesTogether(CurrPose.Pose, PrevPose.Pose, BlendAlpha, OutPose.Pose);
#pragma endregion

}
void ULuaScriptAnimInstance::SetAnimation(UAnimSequence* NewAnim, float BlendingTime, bool LoopAnim, bool ReverseAnim)
{
    if (CurrAnim == NewAnim)
    {
        return;
    }

    if (!PrevAnim && !CurrAnim)
    {
        PrevAnim = NewAnim;
        CurrAnim = NewAnim;
        ElapsedTime = 0.0f;
        PreElapsedTime = 0.0f;
        BlendAlpha = 1.0f;
        bIsBlending = false;
        return;
    }

    if (PrevAnim == nullptr)
    {
        PrevAnim = CurrAnim;
    }
    else if (CurrAnim)
    {
        PrevAnim = CurrAnim;
    }

    CurrAnim = NewAnim;
    BlendDuration = BlendingTime;
    bLooping = LoopAnim;
    bReverse = ReverseAnim;

    // 🛠 Blend 시작을 명확하게 0부터
    BlendStartTime = 0.0f;
    ElapsedTime = 0.0f;
    PreElapsedTime = 0.0f;

    BlendAlpha = 0.0f;
    bIsBlending = true;
    bPlaying = true;
}
