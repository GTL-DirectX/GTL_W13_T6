#pragma once
#pragma comment(lib, "xinput.lib")

#include "RawInput.h"
#include "Delegates/DelegateCombination.h"
#include "HAL/PlatformType.h"
#include "InputCore/InputCoreTypes.h"
#include "Math/Vector.h"
#include "SlateCore/Input/Events.h"
#include "xinput.h"

namespace EMouseButtons
{
enum Type : uint8;
}

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKeyCharDelegate, const TCHAR /*Character*/, const bool /*IsRepeat*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKeyDownDelegate, const FKeyEvent& /*InKeyEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnKeyUpDelegate, const FKeyEvent& /*InKeyEvent*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMouseDownDelegate, const FPointerEvent& /*InMouseEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMouseUpDelegate, const FPointerEvent& /*InMouseEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMouseDoubleClickDelegate, const FPointerEvent& /*InMouseEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMouseWheelDelegate, const FPointerEvent& /*InMouseEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMouseMoveDelegate, const FPointerEvent& /*InMouseEvent*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnControllerButtonDownDelegate, const FControllerButtonEvent& /*InControllerEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnControllerButtonUpDelegate, const FControllerButtonEvent& /*InControllerEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnControllerAnalogDelegate, const FControllerAnalogEvent& /*InControllerEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnControllerConnectedDelegate, uint32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnControllerDisconnectedDelegate, uint32);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRawMouseInputDelegate, const FPointerEvent& /*InRawMouseEvent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRawKeyboardInputDelegate, const FKeyEvent& /*InRawKeyboardEvent*/);

DECLARE_MULTICAST_DELEGATE(FOnPIEModeStart);
DECLARE_MULTICAST_DELEGATE(FOnPIEModeEnd);

class FSlateAppMessageHandler
{
public:
    FSlateAppMessageHandler();

    void ProcessMessage(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam);
    void UpdateXboxControllers(float DeltaTime);
    
    /** Cursor와 관련된 변수를 업데이트 합니다. */
    void UpdateCursorPosition(const FVector2D& NewPos);

    /** 현재 마우스 포인터의 위치를 가져옵니다. */
    FVector2D GetCursorPos() const;

    /** 한 프레임 전의 마우스 포인터의 위치를 가져옵니다. */
    FVector2D GetLastCursorPos() const;

    /** ModifierKeys의 상태를 가져옵니다. */
    FModifierKeysState GetModifierKeys() const;

    void OnPIEModeStart();
    void OnPIEModeEnd();

protected:
    void OnKeyChar(const TCHAR Character, const bool IsRepeat);
    void OnKeyDown(uint32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);
    void OnKeyUp(uint32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);
    
    void OnMouseDown(const EMouseButtons::Type Button, const FVector2D CursorPos);
    void OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos);
    void OnMouseDoubleClick(const EMouseButtons::Type Button, const FVector2D CursorPos);
    void OnMouseWheel(const float Delta, const FVector2D CursorPos);
    void OnMouseMove();

    // Xbox 컨트롤러 함수
    bool IsXboxControllerConnected(uint32 ControllerId) const;
    
    void OnXboxControllerButtonDown(uint32 ControllerId, EXboxButtons::Type Button, bool bIsRepeat = false);
    void OnXboxControllerButtonUp(uint32 ControllerId, EXboxButtons::Type Button);
    void OnXboxControllerAnalogInput(uint32 ControllerId, EXboxAnalog::Type AnalogType, float Value);
    void OnXboxControllerConnected(uint32 ControllerId);
    void OnXboxControllerDisconnected(uint32 ControllerId);
    void SetXboxControllerVibration(uint32 ControllerId, float LeftMotor, float RightMotor);


    // Raw Input
    void OnRawMouseInput(const RAWMOUSE& RawMouseInput);
    void OnRawKeyboardInput(const RAWKEYBOARD& RawKeyboardInput);

    void ProcessXboxControllerButtons(uint32 ControllerId);
    void ProcessXboxControllerAnalogInputs(uint32 ControllerId);
    
    float NormalizeThumbstick(SHORT Value, SHORT DeadZone) const;
    float NormalizeTrigger(BYTE Value) const;

    // 추가적인 함수는 UnrealEngine [SlateApplication.h:1628]을 참조

public:
    FOnKeyCharDelegate OnKeyCharDelegate;
    FOnKeyDownDelegate OnKeyDownDelegate;
    FOnKeyUpDelegate OnKeyUpDelegate;
    
    FOnMouseDownDelegate OnMouseDownDelegate;
    FOnMouseUpDelegate OnMouseUpDelegate;
    FOnMouseDoubleClickDelegate OnMouseDoubleClickDelegate;
    FOnMouseWheelDelegate OnMouseWheelDelegate;
    FOnMouseMoveDelegate OnMouseMoveDelegate;

    // Xbox 컨트롤러 델리게이트들
    FOnControllerButtonDownDelegate OnControllerButtonDownDelegate;
    FOnControllerButtonUpDelegate OnControllerButtonUpDelegate;
    FOnControllerConnectedDelegate OnXboxControllerConnectedDelegate;
    FOnControllerDisconnectedDelegate OnXboxControllerDisconnectedDelegate;
    FOnControllerAnalogDelegate OnXboxControllerAnalogDelegate;

    FOnRawMouseInputDelegate OnRawMouseInputDelegate;
    FOnRawKeyboardInputDelegate OnRawKeyboardInputDelegate;

    FOnPIEModeStart OnPIEModeStartDelegate;
    FOnPIEModeEnd OnPIEModeEndDelegate;

private:
    struct EModifierKey
    {
        enum Type : uint8
        {
            LeftShift,    // VK_LSHIFT
            RightShift,   // VK_RSHIFT
            LeftControl,  // VK_LCONTROL
            RightControl, // VK_RCONTROL
            LeftAlt,      // VK_LMENU
            RightAlt,     // VK_RMENU
            LeftWin,      // VK_LWIN
            RightWin,     // VK_RWIN
            CapsLock,     // VK_CAPITAL
            Count,
        };
    };

    // Cursor Position
    FVector2D CurrentPosition;
    FVector2D PreviousPosition;

    bool ModifierKeyState[EModifierKey::Count];
    TSet<EKeys::Type> PressedMouseButtons;

    std::unique_ptr<FRawInput> RawInputHandler;

private:
    void HandleRawInput(const RAWINPUT& RawInput);

    // Xbox 컨트롤러 관련 멤버 변수들
    static constexpr uint32 MaxControllers = 4;
    XINPUT_STATE XboxControllerStates[MaxControllers];
    XINPUT_STATE XboxPreviousStates[MaxControllers];
    bool XboxControllerConnected[MaxControllers];
    float XboxControllerUpdateTimer;
    static constexpr float XboxControllerUpdateInterval = 1.0f / 60.0f; // 60Hz 업데이트
};
