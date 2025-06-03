#include "InGamePanel.h"

#include "Engine/EditorEngine.h"
#include "GameFramework/GameMode.h"
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

InGamePanel::InGamePanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PIE);
    m_AnimationTime = 0.0f;
    m_RestartHoverState = 0.0f;
    m_ExitHoverState = 0.0f;
    Width = 0.0f;
    Height = 0.0f;
}

void InGamePanel::Render()
{
    // 애니메이션 타이머 업데이트
    m_AnimationTime += ImGui::GetIO().DeltaTime;
    
    // 전체 화면 크기로 패널 설정 (세로 중앙 정렬을 위해)
    const float PanelWidth = Width;   // 전체 화면 너비
    const float PanelHeight = Height; // 전체 화면 높이

    const float PanelPosX = 0.0f;
    const float PanelPosY = 0.0f;

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size - 전체 화면 크기 */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);
    
    // 반투명 패널 스타일 설정
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse |
                                   ImGuiWindowFlags_NoCollapse;

    // 그래디언트 배경 효과를 위한 반투명 배경
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f)); // 더 어두운 배경
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // 자식 윈도우는 투명
    
    if (GEngine && GEngine->ActiveWorld->GetGameMode() && GEngine->ActiveWorld->GetGameMode()->GetIsGameEnded())
    {
        if (ImGui::Begin("InGameRestartPanel", nullptr, window_flags))
        {
            RenderGameModeButton();
        }
        ImGui::End();
    }
    
    // 스타일 색상 복원
    ImGui::PopStyleColor(2);
}

void InGamePanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void InGamePanel::RenderGameModeButton()
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }
    
    // 윈도우 크기 가져오기
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // 컨텐츠 설정 (Game Over가 더 커져서 높이 증가)
    constexpr float titleHeight = 180.0f; // Game Over가 더 커진 만큼 증가
    constexpr float scoreLineHeight = 60.0f; // 텍스트 2배 크기에 맞게 증가
    constexpr float buttonHeight = 80.0f;    // 버튼도 더 크게
    constexpr float buttonWidth = 220.0f;    // 버튼 너비도 증가
    constexpr float spacing = 30.0f;         // 간격도 증가
    constexpr float buttonSpacing = 40.0f;   // 버튼 간격도 증가
    
    // 플레이어 수 계산 및 점수 정렬 (MAX_PLAYER 대신 적절한 상수 사용)
    constexpr int MAX_PLAYERS = 4; // 또는 프로젝트에서 정의된 값 사용
    std::vector<std::pair<int, float>> playerScores;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        APlayer* Player = EditorEngine->GetPIEWorldContext()->World()->GetPlayer(i);
        if (Player)
        {
            playerScores.push_back({i + 1, Player->GetScore()});
        }
    }
    
    // 점수로 정렬 (높은 점수부터)
    std::sort(playerScores.begin(), playerScores.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // 전체 컨텐츠 높이 계산
    float totalContentHeight = titleHeight + 
                              (playerScores.size() * scoreLineHeight) + 
                              spacing * 3 + 
                              buttonHeight;
    
    // 시작 Y 위치 계산 (중앙 정렬)
    float startY = (windowSize.y - totalContentHeight) * 0.5f;
    float currentY = startY;
    
    // 배경 카드 그리기
    DrawBackgroundCard(windowSize, startY, totalContentHeight);
    
    // === 제목 ===
    currentY += 40.0f; // 카드 내 여백 더 증가 (Game Over가 더 커진 만큼)
    RenderAnimatedTitle(windowSize, currentY);
    currentY += titleHeight;
    
    // === 플레이어 점수 (순위별) ===
    RenderPlayerScores(playerScores, windowSize, currentY, scoreLineHeight);
    currentY += playerScores.size() * scoreLineHeight + spacing;
    
    // === 버튼들 ===
    RenderStyledButtons(windowSize, currentY, buttonWidth, buttonHeight, buttonSpacing);
}

void InGamePanel::DrawBackgroundCard(const ImVec2& windowSize, float startY, float contentHeight)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    
    // 카드 크기 계산 (Game Over가 더 커져서 카드도 더 크게)
    float cardWidth = std::min(windowSize.x * 0.95f, 1000.0f); // 더 넓게
    float cardHeight = contentHeight + 80.0f; // 더 높게
    
    ImVec2 card_min = ImVec2(
        window_pos.x + (windowSize.x - cardWidth) * 0.5f,
        window_pos.y + startY - 40.0f // 더 많은 여백
    );
    ImVec2 card_max = ImVec2(card_min.x + cardWidth, card_min.y + cardHeight);
    
    // 그림자 효과
    ImVec2 shadow_min = ImVec2(card_min.x + 12, card_min.y + 12); // 더 큰 그림자
    ImVec2 shadow_max = ImVec2(card_max.x + 12, card_max.y + 12);
    draw_list->AddRectFilled(shadow_min, shadow_max, IM_COL32(0, 0, 0, 130), 25.0f);
    
    // 메인 카드 배경 (그래디언트 효과)
    draw_list->AddRectFilledMultiColor(
        card_min, card_max,
        IM_COL32(45, 45, 65, 240),  // 좌상단
        IM_COL32(35, 35, 55, 240),  // 우상단
        IM_COL32(25, 25, 45, 240),  // 우하단
        IM_COL32(35, 35, 55, 240)   // 좌하단
    );
    
    // 카드 테두리 (글로우 효과)
    draw_list->AddRect(card_min, card_max, IM_COL32(100, 150, 255, 150), 25.0f, 0, 4.0f); // 더 두꺼운 테두리
    draw_list->AddRect(
        ImVec2(card_min.x - 3, card_min.y - 3), 
        ImVec2(card_max.x + 3, card_max.y + 3), 
        IM_COL32(100, 150, 255, 80), 25.0f, 0, 2.5f
    );
}

void InGamePanel::RenderAnimatedTitle(const ImVec2& windowSize, float& currentY)
{
    // 깜빡이는 효과 (on/off)
    float blinkTime = fmod(m_AnimationTime, 1.0f); // 1초 주기
    bool isVisible = blinkTime < 0.7f; // 0.7초 켜지고 0.3초 꺼짐
    
    if (isVisible)
    {
        // 제목 텍스트를 크게 만들기 위한 스케일 설정 (2배 더 크게)
        const float titleScale = 5.0f; // 2.5f에서 5.0f로 증가
        ImGui::SetWindowFontScale(titleScale);
        
        // 제목 텍스트 크기 측정 및 중앙 정렬
        const char* titleText = "GAME OVER";
        ImVec2 titleSize = ImGui::CalcTextSize(titleText);
        float titleX = (windowSize.x - titleSize.x) * 0.5f;
        
        // 글로우 효과 (여러 레이어, 펄스 효과 제거)
        for (int layer = 3; layer >= 0; layer--)
        {
            float alpha = (4 - layer) * 0.2f;
            ImVec4 glowColor = ImVec4(1.0f, 0.3f, 0.3f, alpha); // 펄스 효과 제거
            
            ImGui::PushStyleColor(ImGuiCol_Text, glowColor);
            for (int dx = -layer; dx <= layer; dx++)
            {
                for (int dy = -layer; dy <= layer; dy++)
                {
                    if (dx == 0 && dy == 0 && layer > 0) continue;
                    ImGui::SetCursorPos(ImVec2(titleX + dx * 3, currentY + dy * 3)); // 더 두꺼운 글로우
                    ImGui::Text(titleText);
                }
            }
            ImGui::PopStyleColor();
        }
        
        // 메인 제목 텍스트 (그래디언트 효과를 위한 색상)
        ImGui::SetCursorPos(ImVec2(titleX, currentY));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.9f, 1.0f));
        ImGui::Text(titleText);
        ImGui::PopStyleColor();
        
        // 폰트 스케일 원복
        ImGui::SetWindowFontScale(1.0f);
    }
}

void InGamePanel::RenderPlayerScores(const std::vector<std::pair<int, float>>& playerScores, 
                                   const ImVec2& windowSize, float& currentY, float lineHeight)
{
    const char* rankTexts[] = {"1st", "2nd", "3rd", "4th"};
    const ImVec4 rankColors[] = {
        ImVec4(1.0f, 0.8f, 0.0f, 1.0f),  // 금색
        ImVec4(0.8f, 0.8f, 0.8f, 1.0f),  // 은색
        ImVec4(0.8f, 0.5f, 0.2f, 1.0f),  // 동색
        ImVec4(0.6f, 0.8f, 1.0f, 1.0f)   // 하늘색
    };
    
    // 점수 텍스트 크기를 2배로 설정
    ImGui::SetWindowFontScale(2.0f);
    
    for (size_t i = 0; i < playerScores.size(); i++)
    {
        int playerNum = playerScores[i].first;
        float score = playerScores[i].second;
        
        // 순위 표시를 위한 텍스트와 색상
        const char* rankText = (i < 4) ? rankTexts[i] : "Rank";
        ImVec4 textColor = (i < 4) ? rankColors[i] : ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        
        // 점수 포맷팅
        char scoreBuffer[128];
        sprintf(scoreBuffer, "%s - Player %d: %.3f sec", rankText, playerNum, score);
        
        // 텍스트 크기 측정 및 중앙 정렬 (2배 크기로 계산됨)
        ImVec2 scoreSize = ImGui::CalcTextSize(scoreBuffer);
        float scoreX = (windowSize.x - scoreSize.x) * 0.5f;
        
        // 1등에게 특별한 효과 (글로우 효과 제거, 깔끔한 텍스트)
        if (i == 0)
        {
            // 간단한 테두리만 추가 (번짐 없이)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // 검은색 테두리
            for (int dx = -2; dx <= 2; dx++)
            {
                for (int dy = -2; dy <= 2; dy++)
                {
                    if (dx == 0 && dy == 0) continue;
                    ImGui::SetCursorPos(ImVec2(scoreX + dx, currentY + dy));
                    ImGui::Text(scoreBuffer);
                }
            }
            ImGui::PopStyleColor();
        }
        else
        {
            // 일반 테두리 효과 (더 두껍게)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
            for (int dx = -2; dx <= 2; dx++)
            {
                for (int dy = -2; dy <= 2; dy++)
                {
                    if (dx == 0 && dy == 0) continue;
                    ImGui::SetCursorPos(ImVec2(scoreX + dx, currentY + dy));
                    ImGui::Text(scoreBuffer);
                }
            }
            ImGui::PopStyleColor();
        }
        
        // 메인 텍스트
        ImGui::SetCursorPos(ImVec2(scoreX, currentY));
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::Text(scoreBuffer);
        ImGui::PopStyleColor();
        
        currentY += lineHeight;
    }
    
    // 폰트 스케일 원복
    ImGui::SetWindowFontScale(1.0f);
}

void InGamePanel::RenderStyledButtons(const ImVec2& windowSize, float currentY, 
                                    float buttonWidth, float buttonHeight, float buttonSpacing)
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    
    // 두 버튼의 총 너비 계산
    float totalButtonWidth = (buttonWidth * 2) + buttonSpacing;
    float buttonStartX = (windowSize.x - totalButtonWidth) * 0.5f;
    
    // 버튼 텍스트 크기를 2배로 설정
    ImGui::SetWindowFontScale(2.0f);
    
    // === Restart 버튼 ===
    ImGui::SetCursorPos(ImVec2(buttonStartX, currentY));
    
    // 버튼 호버 상태 체크
    ImVec2 buttonPos = ImGui::GetCursorScreenPos();
    ImVec2 mousePos = ImGui::GetMousePos();
    bool isRestartHovered = (mousePos.x >= buttonPos.x && mousePos.x <= buttonPos.x + buttonWidth &&
                           mousePos.y >= buttonPos.y && mousePos.y <= buttonPos.y + buttonHeight);
    
    // 호버 애니메이션
    m_RestartHoverState = Lerp(m_RestartHoverState, isRestartHovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
    
    // 다이나믹 색상 계산
    float pulseRestart = 0.8f + 0.2f * sin(m_AnimationTime * 2.0f);
    ImVec4 restartBase = ImVec4(0.2f, 0.7f, 0.3f, pulseRestart);
    ImVec4 restartHoverColor = ImVec4(0.3f, 0.9f, 0.4f, 1.0f);
    ImVec4 restartActive = ImVec4(0.1f, 0.6f, 0.2f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, restartBase);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, restartHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, restartActive);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    
    if (ImGui::Button("Restart Game", ImVec2(buttonWidth, buttonHeight)))
    {
        EditorEngine->EndPIE();
        EditorEngine->StartPIE();
    }
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    
    // === Exit 버튼 ===
    ImGui::SameLine(0, buttonSpacing);
    
    // Exit 버튼 호버 상태 체크
    buttonPos = ImGui::GetCursorScreenPos();
    bool isExitHovered = (mousePos.x >= buttonPos.x && mousePos.x <= buttonPos.x + buttonWidth &&
                        mousePos.y >= buttonPos.y && mousePos.y <= buttonPos.y + buttonHeight);
    
    m_ExitHoverState = Lerp(m_ExitHoverState, isExitHovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
    
    // Exit 버튼 색상
    ImVec4 exitBase = ImVec4(0.7f, 0.2f, 0.2f, 0.9f);
    ImVec4 exitHoverColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
    ImVec4 exitActive = ImVec4(0.6f, 0.1f, 0.1f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, exitBase);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, exitHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, exitActive);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    
    if (ImGui::Button("Exit Game", ImVec2(buttonWidth, buttonHeight)))
    {
        EditorEngine->EndPIE();
    }
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    
    // 폰트 스케일 원복
    ImGui::SetWindowFontScale(1.0f);
    
    // 툴팁
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("End current PIE session");
    }
}
