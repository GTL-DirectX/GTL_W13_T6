#pragma once

#include <vector>
#include <algorithm>

#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

class InGamePanel : public UEditorPanel
{
public:
    InGamePanel();
    
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderGameModeButton();
    
    // 새로운 스타일 렌더링 함수들
    void DrawBackgroundCard(const ImVec2& windowSize, float startY, float contentHeight);
    void RenderAnimatedTitle(const ImVec2& windowSize, float& currentY);
    void RenderPlayerScores(const std::vector<std::pair<int, float>>& playerScores, 
                           const ImVec2& windowSize, float& currentY, float lineHeight);
    void RenderStyledButtons(const ImVec2& windowSize, float currentY, 
                           float buttonWidth, float buttonHeight, float buttonSpacing);
    
    // 유틸리티 함수
    float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    
    // 멤버 변수들
    float m_AnimationTime;
    float Width;
    float Height;
    
    // 버튼 애니메이션 상태
    float m_RestartHoverState;
    float m_ExitHoverState;
};
