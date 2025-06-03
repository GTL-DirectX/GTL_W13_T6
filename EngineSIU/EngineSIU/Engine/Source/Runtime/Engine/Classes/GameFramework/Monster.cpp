#include "Monster.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Classes/Particles/ParticleSystemComponent.h"
#include "Classes/Particles/ParticleSystem.h"
#include "SoundManager.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Components/BoxComponent.h"

#include "Components/PrimitiveComponent.h"
#include "UObject/Casts.h"
#include "Player.h"
#include "Engine/EditorEngine.h"

class ULuaScriptAnimInstance;

AMonster::AMonster()
{
    StateToAnimName.Add("Hit", "Armature|Bowser_Hit3");
    StateToAnimName.Add("Falling", "Armature|Bowser_Falling");
    StateToAnimName.Add("Dead", "Armature|Bowser_Backhit");
    StateToAnimName.Add("Land", "Armature|Bowser_Land");
    StateToAnimName.Add("Spin", "Armature|Bowser_Spin");
    StateToAnimName.Add("Roar", "Armature|Bowser_Roar");

}

UObject* AMonster::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewActor;
}

void AMonster::PostSpawnInitialize()
{
    TargetDistributionVector = FDistributionVector(FVector(-110.0f, -110.0f, 0.0f), FVector(110.0f, 110.0f, 0.f));
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);

    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(FName("Contents/Bowser/Bowser_Hit")));
    /*SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    SkeletalMeshComponent->SetAnimClass(UClass::FindClass(FName("ULuaScriptAnimInstance")));*/
    SkeletalMeshComponent->SetStateMachineFileName(StateMachineFileName);

    if (CollisionComponent)
    {
        CollisionComponent->SetBoxExtent(FVector(6.0f, 6.0f, 9.0f));
        CollisionComponent->OnComponentBeginOverlap.AddUObject(this, &ThisClass::OnComponentBeginOverlap);
    }


    /*if (!bNotifyInitialized)
    {*/
    AddMonsterAnimSequenceCache();
    AddAnimNotifies();
    bNotifyInitialized = true;
    //}

    {
        //FootDust
        UParticleSystemComponent* ParticleComponent = AddComponent<UParticleSystemComponent>("ParticleComponent");
        ParticleComponent->SetupAttachment(SkeletalMeshComponent);
        UObject* Object = UAssetManager::Get().GetAsset(EAssetType::ParticleSystem, "Contents/ParticleSystem/FootDust");
        if (UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object))
        {
            ParticleComponent->SetParticleSystem(ParticleSystem);
        }
        ParticleSystemComponentMap["FootDust"] = ParticleComponent;
    }

    {
        //Spin
        UParticleSystemComponent* ParticleComponent = AddComponent<UParticleSystemComponent>("ParticleComponent");
        ParticleComponent->SetupAttachment(SkeletalMeshComponent);
        UObject* Object = UAssetManager::Get().GetAsset(EAssetType::ParticleSystem, "Contents/ParticleSystem/Spin");
        if (UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object))
        {
            ParticleComponent->SetParticleSystem(ParticleSystem);
            ParticleComponent->StopEmissions();
        }
        ParticleSystemComponentMap["Spin"] = ParticleComponent;
    }
    {
        //Hit
        UParticleSystemComponent* ParticleComponent = AddComponent<UParticleSystemComponent>("ParticleComponent");

        ParticleComponent->SetupAttachment(SkeletalMeshComponent);
    
        UObject* Object = UAssetManager::Get().GetAsset(EAssetType::ParticleSystem, "Contents/ParticleSystem/Hit");
        if (UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object))
        {
            ParticleComponent->SetParticleSystem(ParticleSystem);
        }
        ParticleSystemComponentMap["Hit"] = ParticleComponent;
    }
    AttatchParticleComponent();
}

void AMonster::AddMonsterAnimSequenceCache()
{
    const TMap<FName, FAssetInfo> AnimationAssets = UAssetManager::Get().GetAssetRegistry();

    for (const auto& Pair : StateToAnimName)
    {
        FString AssetName = AnimPath + "/" + Pair.Value;
        UAnimationAsset* Animation = UAssetManager::Get().GetAnimation(FName(AssetName));
        UAnimSequence* AnimSeq = nullptr;

        if (!Animation)
        {
            continue;
        }
        AnimSeq = Cast<UAnimSequence>(Animation);
        StateToAnimSequence.Add(Pair.Key, Cast<UAnimSequenceBase>(AnimSeq));
    }
}

void AMonster::AddAnimNotifies()
{
    UAnimSequenceBase* LandingAnimSeq = StateToAnimSequence["Land"];
    int32 TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(LandingAnimSeq, FName(TEXT("Default")));
    if (TrackIdx == INDEX_NONE)
    {
        return;
    }
    int32 NewNotifyIndex = INDEX_NONE;
    float NotifyTime = 0.9f;
    bool bAdded = 
        
        LandingAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnToggleLanding, NewNotifyIndex);
    /*NotifyTime = 0.0f;
    bAdded = LandingAnimSeq->AddDelegateNotifyEventAndBind<AMonster>( TrackIdx, NotifyTime, this, &AMonster::OnToggleLanding, NewNotifyIndex );*/

    if (bAdded)
    {
        printf("[Monster] DelegateNotify 추가 성공: 트랙=%d, 시간=%.3f, 인덱스=%d\n",
            TrackIdx, NotifyTime, NewNotifyIndex);
    }
    else
    {
        printf("[Monster] DelegateNotify 추가 실패\n");
    }


    { //FootDust
        UAnimSequenceBase* LandingAnimSeq = StateToAnimSequence["Land"];
        TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(LandingAnimSeq, FName(TEXT("Default")));

        NewNotifyIndex = INDEX_NONE;
        NotifyTime = 0.005f;
        bAdded = LandingAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnPlayFootDustParticle, NewNotifyIndex);

    {//Spin
        UAnimSequenceBase* SpinAnimSeq = StateToAnimSequence["Spin"];
        TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(SpinAnimSeq, FName(TEXT("Default")));

        NewNotifyIndex = INDEX_NONE;
        NotifyTime = 0.5f;
        bAdded = SpinAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnPlaySpinParticle, NewNotifyIndex);
    }
    }

    UAnimSequenceBase* RoaringAnimSeq = StateToAnimSequence["Roar"];
    TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(RoaringAnimSeq, FName(TEXT("Default")));
    if (TrackIdx == INDEX_NONE)
    {
        return;
    }
    NewNotifyIndex = INDEX_NONE;
    NotifyTime = 0.9f;
    bAdded = RoaringAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnToggleRoaring, NewNotifyIndex);

    NewNotifyIndex = INDEX_NONE;
    NotifyTime = 0.3f;
    bAdded = RoaringAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnPlayRoaringSound, NewNotifyIndex);


    UAnimSequenceBase* DeadAnimSeq = StateToAnimSequence["Dead"];
    TrackIdx = UAnimSequenceBase::EnsureNotifyTrack(DeadAnimSeq, FName(TEXT("Default")));
    if (TrackIdx == INDEX_NONE)
    {
        return;
    }
    NewNotifyIndex = INDEX_NONE;
    NotifyTime = 0.95f;
    bAdded = DeadAnimSeq->AddDelegateNotifyEventAndBind<AMonster>(TrackIdx, NotifyTime, this, &AMonster::OnToggleDead, NewNotifyIndex);




}

void AMonster::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AMonster,
        sol::bases<AActor, ACharacter, APawn>(),
        "GetTargetPosition", &ThisClass::UpdateTargetPosition,
        "UpdateTargetPosition", &ThisClass::UpdateTargetPosition,
        "FollowTimer", sol::property(&ThisClass::GetFollowTimer, &ThisClass::SetFollowTimer),
        "TargetPosition", sol::property(&ThisClass::GetTargetPosition, &ThisClass::SetTargetPosition),
        "IsHit", sol::property(&ThisClass::IsHit, &ThisClass::SetHit),
        "IsDead", sol::property(&ThisClass::IsDead, &ThisClass::SetDead),
        "IsChasing", sol::property(&ThisClass::GetIsChasing, &ThisClass::SetIsChasing),
        "IsLanding", sol::property(&ThisClass::IsLanding, &ThisClass::SetIsLanding),
        "IsFalling", sol::property(&ThisClass::IsFalling, &ThisClass::SetFalling),
        "IsRoaring", sol::property(&ThisClass::IsRoaring, &ThisClass::SetIsRoaring),
        "IsFallingToDeath", sol::property(&ThisClass::IsFallingToDeath, &ThisClass::SetFallingToDeath),
        "GetIsLanding", &ThisClass::IsLanding
    )
        /*DEFINE_LUA_TYPE_NO_PARENT(AMonster,
            "GetTargetPosition", &ThisClass::GetTargetPosition
        )*/
}

bool AMonster::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
    {
        return false;
    }

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName();

    return true;
}

void AMonster::OnDamaged(FVector KnockBackDir)
{
    Super::OnDamaged(KnockBackDir);
    SetFallingToDeath(true);
}

void AMonster::UpdateTargetPosition()
{
    TargetPos = TargetDistributionVector.GetValue();
    TargetDir = TargetPos - GetActorLocation();
}

void AMonster::AttatchParticleComponent()
{
    UParticleSystemComponent* ParticleComp =  ParticleSystemComponentMap["FootDust"];

    if (ParticleComp)
    {
        FVector Pos = FVector(0,0,10);
        FRotator Rot = FRotator();
        FVector Scale = FVector::OneVector;
        FTransform TF = FTransform(Rot, Pos, Scale);
        SkeletalMeshComponent->AddSocket("Hip", "mixamorig:Hip", TF);

        ParticleComp->SetupAttachment(SkeletalMeshComponent);

       
        ParticleComp->SetAttachSocketName("Hip");
    }

}

void AMonster::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    if (!OtherActor || !OtherComp)
    {
        return;
    }
    
    if (APlayer* Character = Cast<APlayer>(OtherActor))
    {
        FVector DamageDir = OtherActor->GetActorLocation() - GetActorLocation();
        Character->OnDamaged(DamageDir);
    }
}


void AMonster::BeginPlay()
{
    Super::BeginPlay();

    if (SkeletalMeshComponent)
    {
        if (ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(SkeletalMeshComponent->GetAnimInstance()))
        {
            if (auto StateMachine = AnimInstance->GetAnimStateMachine())
            {
                StateMachine->BindTargetActor(this);
            }
        }
    }
}
void AMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFalling && GetActorLocation().Z <= 11.7f && (bIsLanding == false && bLandEnded == false))
    {
        // 착지 순간: bIsLanding을 한 번 true로 바꿔 준다.
        bIsLanding = true;
        bFalling = false;
        // bLandEnded는 아직 false, Notify가 붙어 있는 애니메이션이 끝날 때까지 기다린다.
    }

    if (bDead == false && bIsChasing)
    {
        if (TargetDir.Length() < 5.0f)
        {
            UpdateTargetPosition();
        }
        if (GetCapsuleComponent() && GetCapsuleComponent()->BodyInstance)
        {
            auto* CapsuleComp = GetCapsuleComponent();
            CapsuleComp->BodyInstance->AddForce(TargetDir * ChaseSpeed);
        }
    }
}

bool AMonster::IsLanding()
{
    return bIsLanding;
}

bool AMonster::IsFalling() const
{
    return bFalling;
}

bool AMonster::TestToggleVariable() const
{
    static bool bToggle = false;
    bToggle = !bToggle;
    return bToggle;
}



void AMonster::OnToggleLanding(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        // 다른 몬스터의 Notify이므로 무시

        return;
    }
    //UE_LOG(ELogLevel::Error, "Name1 : %s", *MeshComp->GetName());
    //UE_LOG(ELogLevel::Error, "Name2 : %s", *SkeletalMeshComponent->GetName());

    if (bLandEnded == true)
    {
        return;
    }
    bIsLanding = false; // 착지 상태 해제
    bIsRoaring = true;
    //bIsChasing = true; // Roar이후 추격하는 것으로 변경
    bLandEnded = true; // 다시는 위 Notify 실행 X

    //UE_LOG(ELogLevel::Error, TEXT("Monster Landed Name : %s"), *GetName());
}

void AMonster::OnPlayFootDustParticle(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        // 다른 몬스터의 Notify이므로 무시
        return;
    }
    ParticleSystemComponentMap["FootDust"]->StartEmissions();
}

void AMonster::OnPlaySpinParticle(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        // 다른 몬스터의 Notify이므로 무시
        return;
    }
    ParticleSystemComponentMap["Spin"]->StartEmissions();
}

void AMonster::OnToggleRoaring(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        // 다른 몬스터의 Notify이므로 무시
        return;
    }

    if (bRoarEnded == true)
    {
        return;
    }

    bIsRoaring = false;
    bIsChasing = true;

    bRoarEnded = true;
}

void AMonster::OnPlayRoaringSound(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        return;
    }
    FSoundManager::GetInstance().PlaySound("SpawnKoopa");
}

/* Falling, Landing, Roaring 도중엔 Hit 시작되지 않도록 함 */
void AMonster::OnToggleHit(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        // 다른 몬스터의 Notify이므로 무시
        return;
    }
    bIsHit = false; // Hit 상태 해제
    bIsChasing = true; // Hit 이후 추격하는 것으로 변경
    UE_LOG(ELogLevel::Display, TEXT("Monster Name : %s"), *GetName());
}


void AMonster::OnToggleDead(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp != SkeletalMeshComponent)
    {
        return;
    }
    bFallingToDeath = true; // Hit 상태 해제
    UE_LOG(ELogLevel::Error, TEXT("Monster Dead Name : %s"), *GetName());
    //Destroy();
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    /*Engine->ActiveWorld->DestroyActor(this);*/
    //OwnedComponents.Remove(LuaScriptComponent);
    bActorIsBeingDestroyed = true;
}
