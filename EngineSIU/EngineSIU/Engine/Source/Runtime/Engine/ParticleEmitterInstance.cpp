#include "ParticleEmitterInstance.h"

#include <algorithm>
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModule.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/Spawn/ParticleModuleSpawn.h"
#include "UObject/Casts.h"

#include "Engine/FObjLoader.h"
//#include "Particles/ParticleModuleRequired.h"

void FParticleEmitterInstance::Initialize()
{
    CurrentLODLevel = SpriteTemplate->GetLODLevel(CurrentLODLevelIndex);
    
    const TArray<UParticleModule*>& Modules = CurrentLODLevel->GetModules();

    if (CurrentLODLevel->RequiredModule->bUseMaxDrawCount)
    {
        MaxActiveParticles = CurrentLODLevel->CalculateMaxActiveParticleCount();
    }
    else
    {
        MaxActiveParticles = 1000;
    }

    BuildMemoryLayout();

    ParticleData = new uint8[MaxActiveParticles * ParticleStride];
    ParticleIndices = new uint16[MaxActiveParticles];
    InstanceData = new uint8[InstancePayloadSize];

    bIsPlaying = true;
    PlayStartTime = 0.0f;
    ElapsedSincePlay = 0.0f;

    ActiveParticles = 0;
    ParticleCounter = 0;
    AccumulatedTime = 0.0f;
    SpawnFraction = 0.0f;

    bEnabled = true;
}

void FParticleEmitterInstance::Tick(float DeltaTime)
{
    // (1) 엔진 전체 경과 시간 누적 (기존)
    AccumulatedTime += DeltaTime;

    // (2) 재생 중인 경우에만 재생 전용 타이머 갱신
    if (bIsPlaying)
    {
        ElapsedSincePlay += DeltaTime;
    }

    // (3) 파티클 생성 여부 판단: 
    //     - bIsPlaying == false 이면 절대 생성하지 않음
    //     - bIsPlaying == true 이고, 재생 경과 시간이 EmitterDuration 미만인 경우만 생성
    UParticleModuleRequired* ReqModule = CurrentLODLevel->RequiredModule;
    if (!ReqModule)
    {
        return;
    }

    bool bFiniteDuration = (ReqModule->EmitterDuration > 0.0f);
    bool bAllowSpawningThisTick = false;

    if (bIsPlaying)
    {
        if (!bFiniteDuration)
        {
            // Duration이 0 이하(무한)로 설정된 경우, 재생 상태인 한 계속 생성
            bAllowSpawningThisTick = true;
        }
        else
        {
            // 재생 경과 시간이 EmitterDuration 미만일 때만 생성 허용
            if (ElapsedSincePlay < ReqModule->EmitterDuration)
            {
                bAllowSpawningThisTick = true;
            }
            else
            {
                // Duration이 경과하면 자동 중단
                bIsPlaying = false;
                bAllowSpawningThisTick = false;
            }
        }
    }
    // else: bIsPlaying == false -> 생성 금지

    // (4) Burst용 타이머 갱신 (사용자는 재생 중일 때만 Burst가 누적·발생하게 됨)
    if (bIsPlaying)
    {
        CurrentTimeForBurst += DeltaTime;
    }

    // (5) 이번 프레임에 생성할 파티클 개수 계산
    int32 SpawnCount = 0;
    if (bAllowSpawningThisTick)
    {
        SpawnCount = CalculateSpawnCount(DeltaTime);
    }

    // (6) 파티클 스폰
    if (SpawnCount > 0)
    {
        // SpawnTime 계산 시 ElapsedSincePlay를 기준으로 하거나, 
        // 단순히 재생 기준(ElapsedSincePlay 이전값)을 사용한다.
        float PrevElapsed = ElapsedSincePlay - DeltaTime;
        float Increment = (SpawnCount > 1) ? DeltaTime / (float)(SpawnCount - 1) : 0.0f;
        float StartTimeForParticles = PrevElapsed;

        SpawnParticles(SpawnCount, StartTimeForParticles, Increment, FVector::ZeroVector, FVector::ZeroVector);
    }

    // (7) 모듈 및 파티클 업데이트는 생성 여부와 관계없이 항상 수행
    UpdateModules(DeltaTime);
    UpdateParticles(DeltaTime);
}


void FParticleEmitterInstance::SpawnParticles(
    int32 Count, float StartTime, float Increment,
    const FVector& InitialLocation, const FVector& InitialVelocity)
{
    for (int32 i = 0; i < Count; i++)
    {
        int32 NextIndex = ActiveParticles++;
        DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextIndex)

        PreSpawn(Particle, InitialLocation, InitialVelocity);

        int32 Offset = 0;
        float SpawnTime = StartTime + Increment * i;

        UParticleLODLevel* LODLevel = CurrentLODLevel;
        for (auto* Module : LODLevel->GetModules())
        {
            if (Module->bSpawnModule)
            {
                Module->Spawn(this, Offset, SpawnTime, Particle);
            }
        }

        float Interp = (Count > 1) ? (float)i / (float)(Count - 1) : 0.f;
        PostSpawn(Particle, Interp, SpawnTime);
    }
}

void FParticleEmitterInstance::KillParticle(int32 Index)
{
    if (Index < 0 || Index >= ActiveParticles)
    {
        return;
    }

    int32 LastIndex = ActiveParticles - 1;
    if (Index != LastIndex)
    {
        std::memcpy(
            ParticleData + ParticleStride * Index,
            ParticleData + ParticleStride * LastIndex,
            ParticleStride
        );

        // 접근될 일은 없지만 방어적으로 기존 마지막 파티클을 초기화
        std::memset(
            ParticleData + ParticleStride * LastIndex,
            0,
            ParticleStride
        );

        // 아직은 ParticleIndices를 사용하지 않아서 의미는 없지만 추후에 사용될 수 있음
        if (ParticleIndices != nullptr)
        {
            ParticleIndices[Index] = ParticleIndices[LastIndex];
        }
    }
    ActiveParticles--;
}

void FParticleEmitterInstance::StartEmission()
{
    bIsPlaying = true;
    PlayStartTime = 0.0f;
    ElapsedSincePlay = 0.0f;
}

void FParticleEmitterInstance::StopEmission()
{
    bIsPlaying = false;

}

int32 FParticleEmitterInstance::CalculateSpawnCount(float DeltaTime)
{
    UParticleModuleSpawn* SpawnModule = CurrentLODLevel->SpawnModule;

    int32 SpawnCount = 0;

    if (SpawnModule->bProcessBurstList)
    {
        if (CurrentTimeForBurst > SpawnModule->BurstTime)
        {
            SpawnCount = SpawnModule->BurstCount;
            CurrentTimeForBurst = 0.f;
        }
    }
    else
    {
        float Rate = CurrentLODLevel->SpawnModule->Rate.GetValue();
        float RateScale = CurrentLODLevel->SpawnModule->RateScale.GetValue();

        Rate *= RateScale;

        float NewParticlesFloat = Rate * DeltaTime + SpawnFraction;
        SpawnCount = FMath::FloorToInt(NewParticlesFloat);
        SpawnFraction = NewParticlesFloat - SpawnCount;
    }

    return SpawnCount;
}

void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{
    Particle->OldLocation = InitialLocation;
    Particle->Location = InitialLocation;

    Particle->BaseVelocity = InitialVelocity;
    Particle->Velocity = InitialVelocity;

    Particle->BaseRotationRate = 0.f;
    Particle->Rotation = 0.f;
    Particle->RotationRate = 0.f;

    Particle->BaseSize = FVector(1.f, 1.f, 1.f);
    Particle->Size = Particle->BaseSize;
    Particle->Flags = 0;

    Particle->Color = Particle->BaseColor;

    Particle->BaseColor = FLinearColor::White;

    Particle->RelativeTime = 0.f;
    Particle->OneOverMaxLifetime = 0.5f; // 나중에 수명 기반 분포에서 설정 가능
    Particle->Placeholder0 = 0.f;
    Particle->Placeholder1 = 0.f;
}

void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float Interp, float SpawnTime)
{
    Particle->OldLocation = Particle->Location;
    Particle->Location   += FVector(Particle->Velocity) * SpawnTime;
}

void FParticleEmitterInstance::UpdateModules(float DeltaTime)
{
    for (auto* Module : CurrentLODLevel->GetModules())
    {
        if (Module->bEnabled == false)
        {
            continue;
        }

        if (Module->bUpdateModule)
        {
            int32 Offset = Module->GetInstancePayloadSize();
            Module->Update(this, Offset, DeltaTime);
        }
        
        if (Module->bFinalUpdateModule)
        {
            int32 Offset = Module->GetInstancePayloadSize();
            Module->FinalUpdate(this, Offset, DeltaTime);
        }
    }
}

void FParticleEmitterInstance::AllKillParticles()
{
    for (int32 i = ActiveParticles - 1; i >= 0; i--)
    {
        KillParticle(i);
    }
    ActiveParticles = 0;
}

void FParticleEmitterInstance::BuildMemoryLayout()
{
    // BaseParticle 헤더 분량
    AccumulatedTime = 0;
    PayloadOffset = sizeof(FBaseParticle);
    ParticleSize = PayloadOffset;
    InstancePayloadSize = 0;
    for (auto* Module : CurrentLODLevel->GetModules())
    {
        Module->SetModulePayloadOffset(ParticleSize);
        ParticleSize += Module->GetModulePayloadSize();

        if (Module->bUpdateModule)
        {
            Module->SetInstancePayloadOffset(InstancePayloadSize);
            InstancePayloadSize += Module->GetInstancePayloadSize();
        }
    }

    ParticleStride = ParticleSize;
}

bool FParticleEmitterInstance::IsDynamicDataRequired()
{
    if (ActiveParticles <= 0 || !SpriteTemplate)
    {
        return false;
    }

    if (CurrentLODLevel == nullptr || CurrentLODLevel->bEnabled == false ||
        ((CurrentLODLevel->RequiredModule->bUseMaxDrawCount == true) && (CurrentLODLevel->RequiredModule->MaxDrawCount == 0)))
    {
        return false;
    }

    if (Component == nullptr)
    {
        return false;
    }
    return true;
}

void FParticleEmitterInstance::UpdateParticles(float DeltaTime)
{
    for (int32 i = 0; i < ActiveParticles; i++)
    {
        DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * i)

        // 상대 시간 업데이트
        Particle->RelativeTime += DeltaTime * Particle->OneOverMaxLifetime;

        // 수명 종료 판단
        if (Particle->RelativeTime >= 1.0f)
        {
            KillParticle(i);
            i--; // 마지막 인덱스를 앞으로 복사했으므로 인덱스 유지
            continue;
        }

        // 위치 업데이트
        Particle->OldLocation = Particle->Location;
        Particle->Velocity = Particle->BaseVelocity;
        Particle->Location += Particle->Velocity * DeltaTime;

        // 회전 업데이트
        Particle->RotationRate = Particle->BaseRotationRate;
        Particle->Rotation += Particle->RotationRate * DeltaTime;

        // 크기 업데이트
        Particle->Size = Particle->BaseSize;
    }
}

bool FParticleEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    // Make sure there is a template present
    if (!SpriteTemplate)
    {
        return false;
    }

    // Allocate it for now, but we will want to change this to do some form
    // of caching
    if (ActiveParticles <= 0 || !bEnabled)
    {
        return false;
    }
    // If the template is disabled, don't return data.
    if ((CurrentLODLevel == nullptr) || (CurrentLODLevel->bEnabled == false))
    {
        return false;
    }

    OutData.eEmitterType = EDynamicEmitterType::DET_Unknown;

    OutData.ActiveParticleCount = ActiveParticles;
    OutData.ParticleStride = ParticleStride;
    // OutData.SortMode = SortMode;

    OutData.Scale = FVector::OneVector;
    if (Component)
    {
        OutData.Scale = FVector(Component->GetComponentTransform().GetScale3D());
    }

    int32 ParticleMemSize = MaxActiveParticles * ParticleStride;
    OutData.DataContainer.Alloc(ParticleMemSize, MaxActiveParticles);

    memcpy(OutData.DataContainer.ParticleData, ParticleData, ParticleMemSize);
    memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, MaxActiveParticles * sizeof(uint16));

    FDynamicSpriteEmitterReplayData* SpriteReplayData = dynamic_cast<FDynamicSpriteEmitterReplayData*>(&OutData);
    if (SpriteReplayData && SpriteTemplate->EmitterType == EDynamicEmitterType::DET_Sprite)
    {
        SpriteReplayData->SubImages_Horizontal = CurrentLODLevel->RequiredModule->SubImagesHorizontal;
        SpriteReplayData->SubImages_Vertical = CurrentLODLevel->RequiredModule->SubImagesVertical;
        SpriteReplayData->SubUVDataOffset = SubUVDataOffset;
        SpriteReplayData->bUseLocalSpace = CurrentLODLevel->RequiredModule->bUseLocalSpace;
    }

    return true;
}

uint32 FParticleEmitterInstance::GetModuleDataOffset(UParticleModule* Module)
{
    if (SpriteTemplate == nullptr)
    {
        return 0;
    }
    
    uint32* Offset = SpriteTemplate->ModuleOffsetMap.Find(Module);
    return (Offset != nullptr) ? *Offset : 0;
}


//////////////////////// SpriteEmitter
FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(bool bSelected)
{
    // It is valid for the LOD level to be NULL here!
    if (IsDynamicDataRequired() == false || !bEnabled)
    {
        return nullptr;
    }

    // Allocate the dynamic data
    FDynamicSpriteEmitterData* NewEmitterData = new FDynamicSpriteEmitterData(CurrentLODLevel->RequiredModule);

    // Now fill in the source data
    if(!FillReplayData( NewEmitterData->Source ) )
    {
        delete NewEmitterData;
        return nullptr;
    }

    // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
    NewEmitterData->Init( bSelected );

    return NewEmitterData;
}

bool FParticleSpriteEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData) // TODO: 필요한 거 주석 해제
{
    if (ActiveParticles <= 0)
    {
        return false;
    }

    // Call parent implementation first to fill in common particle source data
    if(!FParticleEmitterInstance::FillReplayData( OutData ) )
    {
        return false;
    }

    OutData.eEmitterType = EDynamicEmitterType::DET_Sprite;
    SpriteTemplate->EmitterType = EDynamicEmitterType::DET_Sprite;

    FDynamicSpriteEmitterReplayDataBase* NewReplayData = dynamic_cast< FDynamicSpriteEmitterReplayDataBase* >( &OutData );

    NewReplayData->MaterialInterface = CurrentLODLevel->RequiredModule->MaterialInterface;
    NewReplayData->RequiredModule = CurrentLODLevel->RequiredModule->CreateRendererResource();
    NewReplayData->MaterialInterface = CurrentLODLevel->RequiredModule->MaterialInterface;
    
    // NewReplayData->InvDeltaSeconds = (LastDeltaTime > KINDA_SMALL_NUMBER) ? (1.0f / LastDeltaTime) : 0.0f;
    // NewReplayData->LWCTile = ((Component == nullptr) || CurrentLODLevel->RequiredModule->bUseLocalSpace) ? FVector::Zero() : Component->GetLWCTile();

    NewReplayData->MaxDrawCount = CurrentLODLevel->RequiredModule->bUseMaxDrawCount == true ? CurrentLODLevel->RequiredModule->MaxDrawCount : -1;
    // NewReplayData->ScreenAlignment	= CurrentLODLevel->RequiredModule->ScreenAlignment;
    // NewReplayData->bUseLocalSpace = CurrentLODLevel->RequiredModule->bUseLocalSpace;
    // NewReplayData->EmitterRenderMode = SpriteTemplate->EmitterRenderMode;
    // NewReplayData->DynamicParameterDataOffset = DynamicParameterDataOffset;
    // NewReplayData->LightDataOffset = LightDataOffset;
    // NewReplayData->LightVolumetricScatteringIntensity = LightVolumetricScatteringIntensity;
    // NewReplayData->CameraPayloadOffset = CameraPayloadOffset;

    // NewReplayData->SubUVDataOffset = SubUVDataOffset;
    // NewReplayData->SubImages_Horizontal = CurrentLODLevel->RequiredModule->SubImages_Horizontal;
    // NewReplayData->SubImages_Vertical = CurrentLODLevel->RequiredModule->SubImages_Vertical;

    // NewReplayData->MacroUVOverride.bOverride = CurrentLODLevel->RequiredModule->bOverrideSystemMacroUV;
    // NewReplayData->MacroUVOverride.Radius = CurrentLODLevel->RequiredModule->MacroUVRadius;
    // NewReplayData->MacroUVOverride.Position = FVector(CurrentLODLevel->RequiredModule->MacroUVPosition);
        
    // NewReplayData->bLockAxis = false;
    // if (bAxisLockEnabled == true)
    // {
        // NewReplayData->LockAxisFlag = LockAxisFlags;
        // if (LockAxisFlags != EPAL_NONE)
        // {
            // NewReplayData->bLockAxis = true;
        // }
    // }

    // // If there are orbit modules, add the orbit module data
    // if (LODLevel->OrbitModules.Num() > 0)
    // {
    // 	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    // 	UParticleModuleOrbit* LastOrbit = HighestLODLevel->OrbitModules[LODLevel->OrbitModules.Num() - 1];
    // 	check(LastOrbit);
    //
    // 	uint32* LastOrbitOffset = SpriteTemplate->ModuleOffsetMap.Find(LastOrbit);
    // 	NewReplayData->OrbitModuleOffset = *LastOrbitOffset;
    // }

    // NewReplayData->EmitterNormalsMode = CurrentLODLevel->RequiredModule->EmitterNormalsMode;
    // NewReplayData->NormalsSphereCenter = (FVector)CurrentLODLevel->RequiredModule->NormalsSphereCenter;
    // NewReplayData->NormalsCylinderDirection = (FVector)CurrentLODLevel->RequiredModule->NormalsCylinderDirection;

    // NewReplayData->PivotOffset = FVector2D(PivotOffset);

    // NewReplayData->bUseVelocityForMotionBlur = CurrentLODLevel->RequiredModule->ShouldUseVelocityForMotionBlur();
    // NewReplayData->bRemoveHMDRoll = CurrentLODLevel->RequiredModule->bRemoveHMDRoll;
    // NewReplayData->MinFacingCameraBlendDistance = CurrentLODLevel->RequiredModule->MinFacingCameraBlendDistance;
    // NewReplayData->MaxFacingCameraBlendDistance = CurrentLODLevel->RequiredModule->MaxFacingCameraBlendDistance;

    return true;
}

FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(bool bSelected)
{
    // It is valid for the LOD level to be NULL here!
    if (IsDynamicDataRequired() == false || !bEnabled)
    {
        return nullptr;
    }

    // Allocate the dynamic data
    FDynamicMeshEmitterData* NewEmitterData = new FDynamicMeshEmitterData(CurrentLODLevel->RequiredModule);

    // Now fill in the source data
    if (!FillReplayData(NewEmitterData->Source))
    {
        delete NewEmitterData;
        return nullptr;
    }

    // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
    NewEmitterData->StaticMesh = FObjManager::CreateStaticMesh("Contents/Cube/cube-tex.obj");

    return NewEmitterData;
}

bool FParticleMeshEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    if (ActiveParticles <= 0)
    {
        return false;
    }

    // Call parent implementation first to fill in common particle source data
    if (!FParticleEmitterInstance::FillReplayData(OutData))
    {
        return false;
    }

    OutData.eEmitterType = EDynamicEmitterType::DET_Mesh;
    SpriteTemplate->EmitterType = EDynamicEmitterType::DET_Mesh;

    FDynamicMeshEmitterReplayData* NewReplayData = dynamic_cast<FDynamicMeshEmitterReplayData*>(&OutData);

    NewReplayData->MaterialInterface = CurrentLODLevel->RequiredModule->MaterialInterface;
    NewReplayData->RequiredModule = CurrentLODLevel->RequiredModule->CreateRendererResource();
    NewReplayData->MaterialInterface = CurrentLODLevel->RequiredModule->MaterialInterface;

    // NewReplayData->InvDeltaSeconds = (LastDeltaTime > KINDA_SMALL_NUMBER) ? (1.0f / LastDeltaTime) : 0.0f;
    // NewReplayData->LWCTile = ((Component == nullptr) || CurrentLODLevel->RequiredModule->bUseLocalSpace) ? FVector::Zero() : Component->GetLWCTile();

    NewReplayData->MaxDrawCount = CurrentLODLevel->RequiredModule->bUseMaxDrawCount == true ? CurrentLODLevel->RequiredModule->MaxDrawCount : -1;

    return true;
}
