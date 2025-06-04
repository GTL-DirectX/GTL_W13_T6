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
    InitGame();

    if (FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler())
    {
        /*Handler->OnPIEModeStartDelegate.AddLambda([this]()
        {
            this->InitGame();
        });*/
        // Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
        //     {
        //         // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
        //         if (KeyEvent.GetKeyCode() == VK_SPACE &&
        //             !bGameRunning && bGameEnded)
        //         {
        //             StartMatch();
        //         }
        //     });
        //
        // Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& KeyEvent)
        //     {
        //         // 키가 Space, 아직 게임이 안 시작됐고, 실패 또는 종료되지 않았다면
        //         if (KeyEvent.GetKeyCode() == VK_RCONTROL &&
        //             bGameRunning && !bGameEnded)
        //         {
        //             EndMatch(false);
        //         }
        //     });
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
    if (bGameRunning) return;

    bGameRunning = true;
    bGameEnded = false;
    GameInfo.ElapsedGameTime = 0.0f;
    GameInfo.TotalGameTime = 0.0f;

    /*LuaScriptComponent->GetLuaSelfTable()["ElapsedTime"] = GameInfo.ElapsedGameTime;
    LuaScriptComponent->ActivateFunction("StartMatch");*/

    if (LuaScriptComponent)
    {
        LuaScriptComponent->InitializeComponent();
        LuaScriptComponent->ActivateFunction("OnGameStart");
    }
    FSoundManager::GetInstance().StopAllSounds();
    FSoundManager::GetInstance().PlaySound("BGM");

    float Distance = 20;
    if (GEngine->ActiveWorld->GetPlayer(0))
    {
        GEngine->ActiveWorld->GetPlayer(0)->SetActorLocation(FVector(-Distance, -Distance, 30));
    }
    if (GEngine->ActiveWorld->GetPlayer(1))
    {
        GEngine->ActiveWorld->GetPlayer(1)->SetActorLocation(FVector(-Distance, Distance, 30));
    }
    if (GEngine->ActiveWorld->GetPlayer(2))
    {
        GEngine->ActiveWorld->GetPlayer(2)->SetActorLocation(FVector(Distance, -Distance, 30));
    }
    if (GEngine->ActiveWorld->GetPlayer(3))
    {
        GEngine->ActiveWorld->GetPlayer(3)->SetActorLocation(FVector(Distance, Distance, 30));
    }
    
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

    StartMatch();
}

void AGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MonsterCount = 0;
    for (auto Monster : TObjectRange<AMonster>())
    {
		if (Monster->IsActorBeingDestroyed())
		{
			continue;
		}

        MonsterCount++;
    }

    if (bGameRunning && !bGameEnded)
    {
        // TODO: 아래 코드에서 DeltaTime을 2로 나누는 이유가?
        GameInfo.ElapsedGameTime += DeltaTime;

        bool bAllPlayersDead = true;
        for (int i=0; i<MAX_PLAYER; i++)
        {
            APlayer* Player = GEngine->ActiveWorld->GetPlayer(i);
            if (Player && Player->GetState() != static_cast<int>(EPlayerState::Dead))
            {
                bAllPlayersDead = false;
                break;
            }
        }
        if (bAllPlayersDead)
        {
            EndMatch(false);
        }
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

void AGameMode::DrawStartUI()
{
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.98f));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);

    auto srv = FEngineLoop::ResourceManager
        .GetTexture(L"Assets/Texture/StartLogo.png")
        .get()->TextureSRV;
    ImTextureID my_texture_id = reinterpret_cast<ImTextureID>(srv);

    ImVec2 img_size = ImVec2(1024.0f, 1024.0f);

    if (ImGui::Begin("##ImageOverlay", nullptr, window_flags))
    {
        // 화면 중앙 기준 좌표 계산
        float baseCenterX = (displaySize.x - img_size.x) * 0.5f;
        float baseCenterY = (displaySize.y - img_size.y) * 0.5f;

           // 이미지 위치
        float imagePosX = baseCenterX  -150;
        float imagePosY = baseCenterY + 50;
        ImGui::SetCursorPosX(imagePosX);
        ImGui::SetCursorPosY(imagePosY);
        ImGui::Image(my_texture_id, img_size,
            ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
            ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

       
        ImGui::End();
    }

    ImGui::PopStyleColor();
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



