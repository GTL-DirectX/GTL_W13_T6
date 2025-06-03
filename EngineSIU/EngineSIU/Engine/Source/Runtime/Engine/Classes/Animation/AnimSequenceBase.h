#pragma once
#include "AnimationAsset.h"
#include "AnimDelegateNotify.h"
#include "AnimTypes.h"
#include "UObject/ObjectFactory.h"

class UAnimDataController;
class UAnimDataModel;
struct FAnimNotifyEvent;
struct FAnimNotifyTrack;
class USkeletalMeshComponent;

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)

public:
    UAnimSequenceBase();
    virtual ~UAnimSequenceBase() override = default;

    TArray<FAnimNotifyEvent> Notifies;

    TArray<FAnimNotifyTrack> AnimNotifyTracks;

    float RateScale;

    bool bLoop;

protected:
    UAnimDataModel* DataModel;

    UAnimDataController* Controller;
    
public:
    virtual float GetPlayLength() const override;
    float GetDuration() const;

    UAnimDataModel* GetDataModel() const;

    UAnimDataController& GetController();

    TArray<FAnimNotifyTrack>& GetAnimNotifyTracks() 
    {
        return AnimNotifyTracks;
    }

    bool AddNotifyTrack(const FName& TrackName, int32& OutNewTrackIndex);
    bool RemoveNotifyTrack(int32 TrackIndexToRemove);
    bool RenameNotifyTrack(int32 TrackIndex, const FName& NewTrackName);
    int32 FindNotifyTrackIndex(const FName& TrackName) const;

    bool AddNotifyEvent(int32 TargetTrackIndex, float Time, float Duration, const FName& NotifyName, int32& OutNewNotifyIndex);
    bool RemoveNotifyEvent(int32 NotifyIndexToRemove);
    bool UpdateNotifyEvent(int32 NotifyIndexToUpdate, float NewTime, float NewDuration, int32 NewTrackIndex, const FName& NewNotifyName = NAME_None);
    FAnimNotifyEvent* GetNotifyEvent(int32 NotifyIndex);
    const FAnimNotifyEvent* GetNotifyEvent(int32 NotifyIndex) const;
    void EvaluateAnimNotifies(const TArray<FAnimNotifyEvent>& Notifies, float CurrentTime, float PreviousTime, float DeltaTime, USkeletalMeshComponent* MeshComp, UAnimSequenceBase* AnimAsset, bool bIsLooping);

    /**
    * UAnimDelegateNotify를 사용하여 NotifyEvent를 추가하고, 동시에 InUserObject->InFunction 을 델리게이트에 바인딩
    *
    * @param TargetTrackIndex  : Notify를 추가할 트랙 인덱스
    * @param Time              : 애니메이션 시간(초 단위)
    * @param InUserObject      : 델리게이트에 바인딩할 객체 포인터 (예: this)
    * @param InFunction        : InUserObject의 멤버 함수 포인터 (시그니처: void(UsualClass::*)(USkeletalMeshComponent*, UAnimSequenceBase*))
    * @param OutNewNotifyIndex : 추가된 FAnimNotifyEvent의 전역 인덱스 (리턴용)
    * @return                  : 성공 여부
    */
    template<typename UserClass>
    bool AddDelegateNotifyEventAndBind(
        int32                               TargetTrackIndex,
        float                               Time,
        UserClass* InUserObject,
        void(UserClass::* InFunction)(USkeletalMeshComponent*, UAnimSequenceBase*),
        int32& OutNewNotifyIndex
    );
    static int32 EnsureNotifyTrack(UAnimSequenceBase* AnimSeq, const FName& TrackName);
    
    virtual void SerializeAsset(FArchive& Ar) override;

private:
    void CreateModel();
};

template <typename UserClass>
bool UAnimSequenceBase::AddDelegateNotifyEventAndBind(
    int32 TargetTrackIndex, float Time, UserClass* InUserObject, void(UserClass::* InFunction)(USkeletalMeshComponent*, UAnimSequenceBase*),
    int32& OutNewNotifyIndex
)
{
    OutNewNotifyIndex = INDEX_NONE;

    // 1) Track 인덱스 유효성 검사
    if (!AnimNotifyTracks.IsValidIndex(TargetTrackIndex) || InUserObject == nullptr || InFunction == nullptr)
    {
        return false;
    }

    // 2) DelegateNotify용 이름, Duration=0
    static const FName DelegateNotifyName(TEXT("DelegateNotify"));
    float Duration = 0.f;

    // 3) 기본 AddNotifyEvent 호출
    bool bAdded = AddNotifyEvent(TargetTrackIndex, Time, Duration, DelegateNotifyName,OutNewNotifyIndex);
    if (!bAdded || OutNewNotifyIndex == INDEX_NONE)
    {
        return false;
    }

    // 4) 새로 생성된 FAnimNotifyEvent 가져오기
    FAnimNotifyEvent* NewEvent = GetNotifyEvent(OutNewNotifyIndex);
    if (NewEvent == nullptr)
    {
        return false;
    }

    // 5) UAnimDelegateNotify 객체 생성 후 연결
    UAnimDelegateNotify* DelegateNotifyObj =
        FObjectFactory::ConstructObject<UAnimDelegateNotify>(this);
    if (DelegateNotifyObj == nullptr)
    {
        return false;
    }
    NewEvent->SetAnimNotify(DelegateNotifyObj);

    // 6) 바인딩: 정적 멀티캐스트 델리게이트에 InUserObject, InFunction 등록
    DelegateNotifyObj->OnNotifyTriggered.AddUObject(InUserObject, InFunction);

    return true;
}
