#pragma once
#include "Actor.h"
#include "Delegates/DelegateCombination.h"

DECLARE_MULTICAST_DELEGATE(FOnGameInit);
DECLARE_MULTICAST_DELEGATE(FOnGameStart);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameEnd, bool);

class UCameraComponent;

struct FGameInfo
{
    float TotalGameTime = 0.0f;
    float ElapsedGameTime = 0.0f;
    uint32 CoinScore = 0;
};

class AGameMode : public AActor
{
    DECLARE_CLASS(AGameMode, AActor)
    
public:
    AGameMode() = default;
    virtual ~AGameMode() override = default;

    // 1. Lua에 클래스 등록 / 2. 멤버 변수 등록
    virtual void PostSpawnInitialize() override;
    virtual void RegisterLuaType(sol::state& Lua) override; 
    virtual bool BindSelfLuaProperties() override;
    
    void InitializeComponent();
    virtual UObject* Duplicate(UObject* InOuter) override;

    //virtual void BeginPlay() override;
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 1. 게임 모드 초기화 / 2. 시작 / 3. 종료
    virtual void InitGame();
    virtual void StartMatch();
    virtual void EndMatch(bool bIsWin);
    virtual void Reset();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void SpawnMonster(const FVector& Location, const FRotator& Rotation);

    void SetIsAllPlayerDead(bool bInIsAllPlayerDead) { bIsAllPlayerDead = bInIsAllPlayerDead; }
    bool GetIsAllPlayerDead() const { return bIsAllPlayerDead; }

    void SetPlayerCount(int32 InPlayerCount) { PlayerCount = InPlayerCount; }
    int32 GetPlayerCount() const { return PlayerCount; }

    void SetMonsterCount(int32 InMonsterCount) { MonsterCount = InMonsterCount; }
    int32 GetMonsterCount() const { return MonsterCount; }

    FOnGameInit OnGameInit;
    FOnGameStart OnGameStart;
    FOnGameEnd OnGameEnd;

    FGameInfo GameInfo;
    
protected:
    UPROPERTY(EditAnywhere, FString, ScriptName, = "LuaScripts/Actors/GameMode.lua")
    UPROPERTY(EditAnywhere, int32, PlayerCount, = 0)
    UPROPERTY(EditAnywhere, int32, MonsterCount, = 0)


    bool bGameRunning = false; // 내부 
    bool bGameEnded = true;

    bool bIsAllPlayerDead = false;

    float LogTimer = 0.f;
    float LogInterval = 1.f;  // 1초마다 로그
};

