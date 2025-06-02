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
    StateMachine->ProcessState();

#pragma region MyAnim
    USkeletalMeshComponent* SkeletalMeshComp = GetSkelMeshComponent();
    if (!PrevAnim || !CurrAnim || !SkeletalMeshComp->GetSkeletalMeshAsset()
        || !SkeletalMeshComp->GetSkeletalMeshAsset()->GetSkeleton() || !bPlaying)
    {
        return;
    }


    UAnimSequence* PlayAnim = CurrAnim;  // “현재 재생중인” 애니메이션으로 결정
    const float PreviousTime = ElapsedTime;
    ElapsedTime += DeltaSeconds * PlayRate * (bReverse ? -1.0f : 1.0f);

    float DeltaPlayTime = DeltaSeconds * PlayRate;
    
    const UAnimDataModel* DataModel = PlayAnim->GetDataModel();
    const int32 FrameRate = DataModel->GetFrameRate();
    const int32 NumberOfFrames = DataModel->GetNumberOfFrames();

    LoopStartFrame = FMath::Clamp(LoopStartFrame, 0, NumberOfFrames - 2);
    LoopEndFrame = FMath::Clamp(LoopEndFrame, LoopStartFrame + 1, NumberOfFrames - 1);
    const float StartTime = static_cast<float>(LoopStartFrame) / static_cast<float>(FrameRate);
    const float EndTime = static_cast<float>(LoopEndFrame) / static_cast<float>(FrameRate);
    
    float AnimLength = PlayAnim->GetPlayLength();
    if (IsLooping())
    {
        if (ElapsedTime > EndTime)
        {
            ElapsedTime = StartTime + FMath::Fmod(ElapsedTime - StartTime, LoopEndFrame);
        }
        else if (ElapsedTime <= StartTime)
        {
            ElapsedTime = EndTime - FMath::Fmod(EndTime - ElapsedTime, LoopEndFrame);
        }
    }

    PlayAnim->EvaluateAnimNotifies(
        PlayAnim->Notifies,
        ElapsedTime,
        PreviousTime,
        DeltaPlayTime,
        SkeletalMeshComp,
        PlayAnim,
        bLooping
    );

    // ▶ 그 다음 Blend 로직
    if (bIsBlending)
    {
        float BlendElapsed = FMath::Abs(ElapsedTime - BlendStartTime);
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

    // ▶ 이후에 포즈 계산 및 블렌딩
    const FReferenceSkeleton& RefSkeleton = this->GetCurrentSkeleton()->GetReferenceSkeleton();
    if (PrevAnim->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum() != RefSkeleton.RawRefBoneInfo.Num()
        || CurrAnim->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum() != RefSkeleton.RawRefBoneInfo.Num())
    {
        return;
    }
    int32 BoneCount = RefSkeleton.RawRefBoneInfo.Num();
    OutPose.Pose.Empty();
    OutPose.Pose.InitBones(BoneCount);

    FPoseContext PrevPose(this);
    FPoseContext CurrPose(this);
    PrevPose.Pose.InitBones(BoneCount);
    CurrPose.Pose.InitBones(BoneCount);
    for (int32 BoneIdx = 0; BoneIdx < BoneCount; ++BoneIdx)
    {
        PrevPose.Pose[BoneIdx] = RefSkeleton.RawRefBonePose[BoneIdx];
        CurrPose.Pose[BoneIdx] = RefSkeleton.RawRefBonePose[BoneIdx];
    }

    FAnimExtractContext ExtractA(ElapsedTime, false);
    FAnimExtractContext ExtractB(ElapsedTime, false);
    PrevAnim->GetAnimationPose(PrevPose, ExtractA);
    CurrAnim->GetAnimationPose(CurrPose, ExtractB);
    FAnimationRuntime::BlendTwoPosesTogether(
        CurrPose.Pose, PrevPose.Pose, BlendAlpha, OutPose.Pose
    );
#pragma endregion
}


void ULuaScriptAnimInstance::SetAnimation(UAnimSequence* NewAnim, float BlendingTime, bool LoopAnim, bool ReverseAnim)
{
    if (CurrAnim == NewAnim)
    {
        return; // 이미 같은 애니메이션이 설정되어 있다면 아무 작업도 하지 않음.
    }

    if (!PrevAnim && !CurrAnim)
    {
        PrevAnim = NewAnim;
        CurrAnim = NewAnim;
    }
    else if (PrevAnim == nullptr)
    {
        PrevAnim = CurrAnim; // 이전 애니메이션이 없으면 현재 애니메이션을 이전으로 설정.
    }
    else if (CurrAnim)
    {
        PrevAnim = CurrAnim; // 현재 애니메이션이 있으면 현재를 이전으로 설정.
    }

    CurrAnim = NewAnim;
    BlendDuration = BlendingTime;
    bLooping = LoopAnim;
    bReverse = ReverseAnim;
    
    //ElapsedTime = 0.0f;
    BlendStartTime = ElapsedTime;
    BlendAlpha = 0.0f;
    bIsBlending = true;
    bPlaying = true;
}
