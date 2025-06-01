#pragma once
#include "Define.h"
#include "Container/Set.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"
#include "WorldType.h"
#include "Level.h"
#include "Actors/EditorPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EventManager.h"
#include "GameFramework/Player.h"
#include "UObject/UObjectIterator.h"

constexpr int MAX_PLAYER = 4;

class ACharacter;
class UPrimitiveComponent;
struct FOverlapResult;
class UCameraComponent;
class FObjectFactory;
class AActor;
class UObject;
class USceneComponent;
class FCollisionManager;
class AGameMode;
class UTextComponent;

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)

public:
    UWorld() = default;
    virtual ~UWorld() override;

    static UWorld* CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName = "DefaultWorld");

    void InitializeNewWorld();
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void Tick(float DeltaTime);
    void BeginPlay();

    void Release();

    /**
     * World에 Actor를 Spawn합니다.
     * @param InClass Spawn할 Actor 정보
     * @return Spawn된 Actor
     */
    AActor* SpawnActor(UClass* InClass, FName InActorName = NAME_None);

    /** 
     * World에 Actor를 Spawn합니다.
     * @tparam T AActor를 상속받은 클래스
     * @return Spawn된 Actor의 포인터
     */
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();

    /** World에 존재하는 Actor를 제거합니다. */
    bool DestroyActor(AActor* ThisActor);

    virtual UWorld* GetWorld() const override;
    ULevel* GetActiveLevel() const { return ActiveLevel; }

    template <typename T>
        requires std::derived_from<T, AActor>
    T* DuplicateActor(T* InActor);

    EWorldType WorldType = EWorldType::None;
    
    FEventManager EventManager;

    void SetPlayer(int Index, APlayer* InPlayer){ Players[Index] = InPlayer; }
    APlayer* GetPlayer(int Index) const;

    void SetPlayerController(int Index, APlayerController* InPlayerController){ PlayerControllers[Index] = InPlayerController; }
    APlayerController* GetPlayerController(int Index) const;

    AGameMode* GetGameMode() const { return GameMode; }

    bool IsPlayerConnected(int Index) const { return bCurrentlyConnected[Index]; };
    
    void CheckOverlap(const UPrimitiveComponent* Component, TArray<FOverlapResult>& OutOverlaps) const;

    void ConnectedPlayer(int Index);
    void DisconnectedPlayer(int Index);
public:
    double TimeSeconds;

protected:
    
    FString WorldName = "DefaultWorld";
    
private:
    AGameMode* GameMode = nullptr;


    ULevel* ActiveLevel;

    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;
    
    bool bCurrentlyConnected[MAX_PLAYER] = { false, };
    APlayerController* PlayerControllers[MAX_PLAYER] = { nullptr, };
    APlayer* Players[MAX_PLAYER] = { nullptr, };

    UTextComponent* MainTextComponent = nullptr;

    FCollisionManager* CollisionManager = nullptr;
};


template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    return Cast<T>(SpawnActor(T::StaticClass()));
}

template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::DuplicateActor(T* InActor)
{
    if (ULevel* ActiveLevel = GetActiveLevel())
    {
        T* NewActor = static_cast<T*>(InActor->Duplicate(this));
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        return NewActor;
    }
    return nullptr;
}

