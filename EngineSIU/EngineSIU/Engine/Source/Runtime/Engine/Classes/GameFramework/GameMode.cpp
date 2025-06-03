#include "GameMode.h"
#include "EngineLoop.h"
#include "Monster.h"
#include "SoundManager.h"
#include "InputCore/InputCoreTypes.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World/World.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"

void AGameMode::PostSpawnInitialize()
{
    AActor::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName(ScriptName);


    OnGameInit.AddLambda([]() { UE_LOG(ELogLevel::Display, TEXT("Game Initialized")); });
    
    SetActorTickInEditor(false); // PIE 모드에서만 Tick 수행

    if (FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler())
    {
        /*Handler->OnPIEModeStartDelegate.AddLambda([this]()
        {
            this->InitGame();
        });*/
        Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
        {
            // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
            if (KeyEvent.GetKeyCode() == VK_SPACE &&
                !bGameRunning && bGameEnded)
            {
                StartMatch();
            }
        });

        Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
            {
                // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
                if (KeyEvent.GetKeyCode() == VK_RCONTROL && 
                    bGameRunning && !bGameEnded)
                {
                    EndMatch(false);
                }
            });
    }
}

void AGameMode::RegisterLuaType(sol::state& Lua)
{
    Super::RegisterLuaType(Lua);

    DEFINE_LUA_TYPE_NO_PARENT(AGameMode,
		"SpawnMonster", &ThisClass::SpawnMonster,
        "IsAllPlayerDead", sol::property(&ThisClass::SetIsAllPlayerDead, &ThisClass::GetIsAllPlayerDead),
        "PlayerCount", sol::property(&ThisClass::SetPlayerCount, &ThisClass::GetPlayerCount),
        "MonsterCount", sol::property(&ThisClass::SetMonsterCount, &ThisClass::GetMonsterCount)
    )
}

bool AGameMode::BindSelfLuaProperties()
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

void AGameMode::InitializeComponent()
{

}

UObject* AGameMode::Duplicate(UObject* InOuter)
{
    AGameMode* NewActor = Cast<AGameMode>(Super::Duplicate(InOuter));

    if (NewActor)
    {
        NewActor->bGameRunning = bGameRunning;
        NewActor->bGameEnded = bGameEnded;
        NewActor->GameInfo = GameInfo;
    }
    return NewActor;
}


void AGameMode::InitGame()
{
    OnGameInit.Broadcast();
}

void AGameMode::StartMatch()
{
    bGameRunning = true;
    bGameEnded = false;
    GameInfo.ElapsedGameTime = 0.0f;
    GameInfo.TotalGameTime = 0.0f;
  
    /*LuaScriptComponent->GetLuaSelfTable()["ElapsedTime"] = GameInfo.ElapsedGameTime;
    LuaScriptComponent->ActivateFunction("StartMatch");*/
    
    OnGameStart.Broadcast();
}

void AGameMode::BeginPlay()
{
    Super::BeginPlay();
    // 게임 모드가 시작되면 Lua 스크립트 컴포넌트 초기화
    if (LuaScriptComponent)
    {
        LuaScriptComponent->InitializeComponent();
        LuaScriptComponent->ActivateFunction("OnGameStart");
    }
    FSoundManager::GetInstance().StopAllSounds();
    FSoundManager::GetInstance().PlaySound("BGM");
}

void AGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bGameRunning && !bGameEnded)
    {
        // TODO: 아래 코드에서 DeltaTime을 2로 나누는 이유가?
        GameInfo.ElapsedGameTime += DeltaTime / 2.0f;
    }
}

void AGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    FSoundManager::GetInstance().StopAllSounds();
}

void AGameMode::SpawnMonster(const FVector& Location, const FRotator& Rotation)
{
    /*if (!bGameRunning || bGameEnded)
    {
        UE_LOG(ELogLevel::Warning, TEXT("Cannot spawn monster when game is not running or has ended."));
        return;
    }*/
    // 몬스터 스폰 로직 구현
    // 예시: 몬스터를 SpawnActor로 생성하고 위치와 회전을 설정
    AMonster* SpawnedMonster = GetWorld()->SpawnActor<AMonster>();
    if (SpawnedMonster)
    {
        SpawnedMonster->SetActorLocation(Location);
        SpawnedMonster->SetActorRotation(Rotation);
        SpawnedMonster->SetActorScale(FVector(1, 1, 1));

        SetMonsterCount(GetMonsterCount() + 1);
    }

    if (SpawnedMonster)
    {
        UE_LOG(ELogLevel::Display, TEXT("Spawned monster at %s"), *Location.ToString());
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to spawn monster"));
    }
}


void AGameMode::EndMatch(bool bIsWin)
{
    // 이미 종료된 상태라면 무시
    if (!bGameRunning || bGameEnded)
    {
        return;
    }

    this->Reset();
    
    GameInfo.TotalGameTime = GameInfo.ElapsedGameTime;
    
    OnGameEnd.Broadcast(bIsWin);
}

void AGameMode::Reset()
{
    bGameRunning = false;
    bGameEnded = true;
    FSoundManager::GetInstance().StopAllSounds();
}



