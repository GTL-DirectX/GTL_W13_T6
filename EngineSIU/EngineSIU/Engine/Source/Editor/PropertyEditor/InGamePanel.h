#pragma once
#include <vector>

#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

class InGamePanel : public UEditorPanel
{

public:
    InGamePanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    // 기존 함수들
    void RenderGameModeButton();
    void DrawBackgroundCard(const ImVec2& windowSize, float startY, float contentHeight);
    void RenderAnimatedTitle(const ImVec2& windowSize, float& currentY);
    void RenderPlayerScores(const std::vector<std::pair<int, float>>& playerScores, 
                           const ImVec2& windowSize, float& currentY, float lineHeight);
    void RenderStyledButtons(const ImVec2& windowSize, float currentY, 
                           float buttonWidth, float buttonHeight, float buttonSpacing);
    
    // 새로 추가된 함수
    void RenderPauseButton();

    // 유틸리티 함수
    inline float Lerp(float a, float b, float t) 
    { 
        return a + t * (b - a); 
    }

private:
    // 애니메이션 및 상태 변수들
    float m_AnimationTime;
    float m_RestartHoverState;
    float m_ExitHoverState;
    float m_PauseHoverState;    // 새로 추가: 정지 버튼 호버 상태
    
    // 화면 크기
    float Width;
    float Height;
};
