#pragma once
#include "AnimSequenceBase.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "UObject/ObjectMacros.h"

struct FPoseContext;
struct FTransform;
struct FBoneAnimationTrack;
struct FAnimationPoseData;
struct FAnimExtractContext;

class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)

public:
    UAnimSequence();
    virtual ~UAnimSequence() override = default;
    
    virtual void GetAnimationPose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext);
    
    void GetBonePose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext);
    
    virtual void SerializeAsset(FArchive& Ar) override;
    
};
