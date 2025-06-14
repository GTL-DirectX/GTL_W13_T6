// ReSharper disable CppMemberFunctionMayBeConst
#include "SlateAppMessageHandler.h"

#define _TCHAR_DEFINED
#include <windowsx.h>

#include "Define.h"
#include "EngineLoop.h"
#include "WindowsCursor.h"
#include "Engine/Engine.h"
#include "Math/Vector.h"
#include "World/World.h"

extern FEngineLoop GEngineLoop;


FSlateAppMessageHandler::FSlateAppMessageHandler()
    : CurrentPosition(FVector2D::ZeroVector)
    , PreviousPosition(FVector2D::ZeroVector)
{
    for (bool& KeyState : ModifierKeyState)
    {
        KeyState = false;
    }

    RawInputHandler = std::make_unique<FRawInput>(GEngineLoop.AppWnd, [this](const RAWINPUT& RawInput)
    {
        HandleRawInput(RawInput);
    });

    // Xbox 컨트롤러 초기화
    for (uint32 i = 0; i < MaxControllers; ++i)
    {
        XboxControllerConnected[i] = false;
    }
}

void FSlateAppMessageHandler::HandleRawInput(const RAWINPUT& RawInput)
{
    if (RawInput.header.dwType == RIM_TYPEMOUSE)
    {
        OnRawMouseInput(RawInput.data.mouse);
    }
    else if (RawInput.header.dwType == RIM_TYPEKEYBOARD)
    {
        OnRawKeyboardInput(RawInput.data.keyboard);
    }
}

void FSlateAppMessageHandler::ProcessMessage(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    // 입력 장치에 변경이 생겼을 때
    case WM_INPUT_DEVICE_CHANGE:
    {
        // RawInputHandler->ReRegisterDevices();
        return;
    }

    case WM_CHAR:
    {
        // WPARAM으로부터 문자 가져오기
        const TCHAR Character = static_cast<TCHAR>(wParam);

        // lParam의 30번째 비트(0x40000000)는 키가 계속 눌려져 있는 상태(키 반복)인지 확인
        const bool bIsRepeat = (lParam & 0x40000000) != 0;
        OnKeyChar(Character, bIsRepeat);
        return;
    }

    // 키보드 키가 눌렸을 때 발생하는 메시지
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        const int32 Win32Key = static_cast<int32>(wParam);
        int32 ActualKey = Win32Key;

        bool bIsRepeat = (lParam & 0x40000000) != 0;

        switch (Win32Key)
        {
        case VK_MENU:
            // Differentiate between left and right alt
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LMENU;
                bIsRepeat = ModifierKeyState[EModifierKey::LeftAlt];
                ModifierKeyState[EModifierKey::LeftAlt] = true;
            }
            else
            {
                ActualKey = VK_RMENU;
                bIsRepeat = ModifierKeyState[EModifierKey::RightAlt];
                ModifierKeyState[EModifierKey::RightAlt] = true;
            }
            break;
        case VK_CONTROL:
            // Differentiate between left and right control
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LCONTROL;
                bIsRepeat = ModifierKeyState[EModifierKey::LeftControl];
                ModifierKeyState[EModifierKey::LeftControl] = true;
            }
            else
            {
                ActualKey = VK_RCONTROL;
                bIsRepeat = ModifierKeyState[EModifierKey::RightControl];
                ModifierKeyState[EModifierKey::RightControl] = true;
            }
            break;
        case VK_SHIFT:
            // Differentiate between left and right shift
            ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (ActualKey == VK_LSHIFT)
            {
                bIsRepeat = ModifierKeyState[EModifierKey::LeftShift];
                ModifierKeyState[EModifierKey::LeftShift] = true;
            }
            else
            {
                bIsRepeat = ModifierKeyState[EModifierKey::RightShift];
                ModifierKeyState[EModifierKey::RightShift] = true;
            }
            break;
        case VK_LWIN:
        case VK_RWIN:
            // Differentiate between left and right window key
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LWIN;
                bIsRepeat = ModifierKeyState[EModifierKey::LeftWin];
                ModifierKeyState[EModifierKey::LeftWin] = true;
            }
            else
            {
                ActualKey = VK_RWIN;
                bIsRepeat = ModifierKeyState[EModifierKey::RightWin];
                ModifierKeyState[EModifierKey::RightWin] = true;
            }
            break;
        case VK_CAPITAL:
            ModifierKeyState[EModifierKey::CapsLock] = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            break;
        default:
            // No translation needed
            break;
        }

        const uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);
        OnKeyDown(ActualKey, CharCode, bIsRepeat);
        return;
    }

    case WM_SYSKEYUP: // 시스템 키(Alt, F10 등)가 눌렸다가 떼어질 때 발생하는 메시지
    case WM_KEYUP:    // 키보드 키가 떼어졌을 때 발생하는 메시지
    {
        // Character code is stored in WPARAM
        const int32 Win32Key = static_cast<int32>(wParam);
        int32 ActualKey = Win32Key;

        switch (Win32Key)
        {
        case VK_MENU:
            // Differentiate between left and right alt
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LMENU;
                ModifierKeyState[EModifierKey::LeftAlt] = false;
            }
            else
            {
                ActualKey = VK_RMENU;
                ModifierKeyState[EModifierKey::RightAlt] = false;
            }
            break;
        case VK_CONTROL:
            // Differentiate between left and right control
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LCONTROL;
                ModifierKeyState[EModifierKey::LeftControl] = false;
            }
            else
            {
                ActualKey = VK_RCONTROL;
                ModifierKeyState[EModifierKey::RightControl] = false;
            }
            break;
        case VK_SHIFT:
            // Differentiate between left and right shift
            ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (ActualKey == VK_LSHIFT)
            {
                ModifierKeyState[EModifierKey::LeftShift] = false;
            }
            else
            {
                ModifierKeyState[EModifierKey::RightShift] = false;
            }
            break;
        case VK_LWIN:
        case VK_RWIN:
            // Differentiate between left and right window key
            if ((lParam & 0x1000000) == 0)
            {
                ActualKey = VK_LWIN;
                ModifierKeyState[EModifierKey::LeftWin] = false;
            }
            else
            {
                ActualKey = VK_RWIN;
                ModifierKeyState[EModifierKey::RightWin] = false;
            }
            break;
        case VK_CAPITAL:
            ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            break;
        default:
            // No translation needed
            break;
        }

        // Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
        const uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);

        // Key up events are never repeats
        constexpr bool bIsRepeat = false;
        OnKeyUp(ActualKey, CharCode, bIsRepeat);
        return;
    }

    // Mouse Button Down
    case WM_LBUTTONDBLCLK: // 마우스 왼쪽 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_LBUTTONDOWN:   // 마우스 왼쪽 버튼이 눌렸을 때 발생하는 메시지
    case WM_MBUTTONDBLCLK: // 마우스 가운데 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_MBUTTONDOWN:   // 마우스 가운데 버튼이 눌렸을 때 발생하는 메시지
    case WM_RBUTTONDBLCLK: // 마우스 오른쪽 버튼이 더블 클릭되었을 때 발생하는 메시지
    case WM_RBUTTONDOWN:   // 마우스 오른쪽 버튼이 눌렸을 때 발생하는 메시지
    case WM_XBUTTONDBLCLK: // 마우스 추가 버튼(X1, X2)이 더블 클릭되었을 때 발생하는 메시지
    case WM_XBUTTONDOWN:   // 마우스 추가 버튼(X1, X2)이 눌렸을 때 발생하는 메시지
    case WM_XBUTTONUP:     // 마우스 추가 버튼(X1, X2)이 떼어졌을 때 발생하는 메시지
    case WM_LBUTTONUP:     // 마우스 왼쪽 버튼이 떼어졌을 때 발생하는 메시지
    case WM_MBUTTONUP:     // 마우스 가운데 버튼이 떼어졌을 때 발생하는 메시지
    case WM_RBUTTONUP:     // 마우스 오른쪽 버튼이 떼어졌을 때 발생하는 메시지
    {
        POINT CursorPoint;
        CursorPoint.x = GET_X_LPARAM(lParam);
        CursorPoint.y = GET_Y_LPARAM(lParam);

        ClientToScreen(hWnd, &CursorPoint);

        const FVector2D CursorPos{
            static_cast<float>(CursorPoint.x),
            static_cast<float>(CursorPoint.y)
        };

        EMouseButtons::Type MouseButton = EMouseButtons::Invalid;
        bool bDoubleClick = false;
        bool bMouseUp = false;
        switch (Msg)
        {
        case WM_LBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Left;
            break;
        case WM_LBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Left;
            break;
        case WM_LBUTTONDOWN:
            MouseButton = EMouseButtons::Left;
            break;
        case WM_MBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_MBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_MBUTTONDOWN:
            MouseButton = EMouseButtons::Middle;
            break;
        case WM_RBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = EMouseButtons::Right;
            break;
        case WM_RBUTTONUP:
            bMouseUp = true;
            MouseButton = EMouseButtons::Right;
            break;
        case WM_RBUTTONDOWN:
            MouseButton = EMouseButtons::Right;
            break;
        case WM_XBUTTONDBLCLK:
            bDoubleClick = true;
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        case WM_XBUTTONUP:
            bMouseUp = true;
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        case WM_XBUTTONDOWN:
            MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
            break;
        default:
            assert(0);
        }

        if (bMouseUp)
        {
            OnMouseUp(MouseButton, CursorPos);
        }
        else if (bDoubleClick)
        {
            OnMouseDoubleClick(MouseButton, CursorPos);
        }
        else
        {
            OnMouseDown(MouseButton, CursorPos);
        }
        return;
    }

    // Mouse Movement
    case WM_NCMOUSEMOVE: // 비클라이언트 영역(창 제목 표시줄 등)에서 마우스가 움직였을 때 발생하는 메시지
    case WM_MOUSEMOVE:   // 클라이언트 영역에서 마우스가 움직였을 때 발생하는 메시지
    {
        UpdateCursorPosition(FWindowsCursor::GetPosition());
        OnMouseMove(); // TODO: UE [WindowsApplication.cpp:2286]
        return;
    }

    // 마우스 휠이 움직였을 때 발생하는 메시지 (올리면 +120, 내리면 -120)
    case WM_MOUSEWHEEL:
    {
        constexpr float SpinFactor = 1 / 120.0f;
        const SHORT WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        POINT CursorPoint;
        CursorPoint.x = GET_X_LPARAM(lParam);
        CursorPoint.y = GET_Y_LPARAM(lParam);

        const FVector2D CursorPos{
            static_cast<float>(CursorPoint.x),
            static_cast<float>(CursorPoint.y)
        };
        OnMouseWheel(static_cast<float>(WheelDelta) * SpinFactor, CursorPos);
        return;
    }

    case WM_INPUT:
    {
        // RawInput을 처리하는 부분
        RawInputHandler->ProcessRawInput(lParam);
        return;
    }

    default:
    {
        // 추후에 추가 Message가 필요하면 다음 파일을 참조
        // UnrealEngine [WindowsApplication.cpp:1990]
        return;
    }
    }
}

void FSlateAppMessageHandler::ProcessXboxControllerButtons(uint32 ControllerId)
{
    if (!IsXboxControllerConnected(ControllerId))
    {
        return;
    }
    
    const XINPUT_GAMEPAD& CurrentGamepad = XboxControllerStates[ControllerId].Gamepad;
    const XINPUT_GAMEPAD& PreviousGamepad = XboxPreviousStates[ControllerId].Gamepad;
    
    // 버튼 매핑 테이블
    struct ButtonMapping
    {
        WORD XInputButton;
        EXboxButtons::Type OurButton;
    };
    
    static const ButtonMapping ButtonMappings[] = {
        { XINPUT_GAMEPAD_A, EXboxButtons::A },
        { XINPUT_GAMEPAD_B, EXboxButtons::B },
        { XINPUT_GAMEPAD_X, EXboxButtons::X },
        { XINPUT_GAMEPAD_Y, EXboxButtons::Y },
        { XINPUT_GAMEPAD_LEFT_SHOULDER, EXboxButtons::LeftBumper },
        { XINPUT_GAMEPAD_RIGHT_SHOULDER, EXboxButtons::RightBumper },
        { XINPUT_GAMEPAD_BACK, EXboxButtons::Back },
        { XINPUT_GAMEPAD_START, EXboxButtons::Start },
        { XINPUT_GAMEPAD_LEFT_THUMB, EXboxButtons::LeftThumbstick },
        { XINPUT_GAMEPAD_RIGHT_THUMB, EXboxButtons::RightThumbstick },
        { XINPUT_GAMEPAD_DPAD_UP, EXboxButtons::DPadUp },
        { XINPUT_GAMEPAD_DPAD_DOWN, EXboxButtons::DPadDown },
        { XINPUT_GAMEPAD_DPAD_LEFT, EXboxButtons::DPadLeft },
        { XINPUT_GAMEPAD_DPAD_RIGHT, EXboxButtons::DPadRight }
    };
    
    // 각 버튼에 대해 상태 변화 확인
    for (const auto& Mapping : ButtonMappings)
    {
        bool bCurrentPressed = (CurrentGamepad.wButtons & Mapping.XInputButton) != 0;
        bool bPreviousPressed = (PreviousGamepad.wButtons & Mapping.XInputButton) != 0;
        
        if (bCurrentPressed && !bPreviousPressed)
        {
            // 버튼이 방금 눌림
            OnXboxControllerButtonDown(ControllerId, Mapping.OurButton, false);
        }
        else if (!bCurrentPressed && bPreviousPressed)
        {
            // 버튼이 방금 떼어짐
            OnXboxControllerButtonUp(ControllerId, Mapping.OurButton);
        }
        else if (bCurrentPressed && bPreviousPressed)
        {
            // 버튼이 계속 눌려있음 (반복)
            OnXboxControllerButtonDown(ControllerId, Mapping.OurButton, true);
        }
    }
    
    // 트리거 버튼 처리 (디지털)
    bool bLeftTriggerPressed = CurrentGamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    bool bLeftTriggerPrevPressed = PreviousGamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    bool bRightTriggerPressed = CurrentGamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    bool bRightTriggerPrevPressed = PreviousGamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    
    if (bLeftTriggerPressed && !bLeftTriggerPrevPressed)
    {
        OnXboxControllerButtonDown(ControllerId, EXboxButtons::LeftTrigger, false);
    }
    else if (!bLeftTriggerPressed && bLeftTriggerPrevPressed)
    {
        OnXboxControllerButtonUp(ControllerId, EXboxButtons::LeftTrigger);
    }
    
    if (bRightTriggerPressed && !bRightTriggerPrevPressed)
    {
        OnXboxControllerButtonDown(ControllerId, EXboxButtons::RightTrigger, false);
    }
    else if (!bRightTriggerPressed && bRightTriggerPrevPressed)
    {
        OnXboxControllerButtonUp(ControllerId, EXboxButtons::RightTrigger);
    }
}

void FSlateAppMessageHandler::ProcessXboxControllerAnalogInputs(uint32 ControllerId)
{
    if (!IsXboxControllerConnected(ControllerId))
    {
        return;
    }
    
    const XINPUT_GAMEPAD& CurrentGamepad = XboxControllerStates[ControllerId].Gamepad;
    const XINPUT_GAMEPAD& PreviousGamepad = XboxPreviousStates[ControllerId].Gamepad;
    
    // 왼쪽 스틱 X축
    float LeftStickX = NormalizeThumbstick(CurrentGamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    float PrevLeftStickX = NormalizeThumbstick(PreviousGamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    if (FMath::Abs(LeftStickX - PrevLeftStickX) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::LeftStickX, LeftStickX);
    }
    
    // 왼쪽 스틱 Y축
    float LeftStickY = NormalizeThumbstick(CurrentGamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    float PrevLeftStickY = NormalizeThumbstick(PreviousGamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    if (FMath::Abs(LeftStickY - PrevLeftStickY) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::LeftStickY, LeftStickY);
    }
    
    // 오른쪽 스틱 X축
    float RightStickX = NormalizeThumbstick(CurrentGamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    float PrevRightStickX = NormalizeThumbstick(PreviousGamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    if (FMath::Abs(RightStickX - PrevRightStickX) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::RightStickX, RightStickX);
    }
    
    // 오른쪽 스틱 Y축
    float RightStickY = NormalizeThumbstick(CurrentGamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    float PrevRightStickY = NormalizeThumbstick(PreviousGamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    if (FMath::Abs(RightStickY - PrevRightStickY) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::RightStickY, RightStickY);
    }
    
    // 왼쪽 트리거
    float LeftTrigger = NormalizeTrigger(CurrentGamepad.bLeftTrigger);
    float PrevLeftTrigger = NormalizeTrigger(PreviousGamepad.bLeftTrigger);
    if (FMath::Abs(LeftTrigger - PrevLeftTrigger) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::LeftTriggerAxis, LeftTrigger);
    }
    
    // 오른쪽 트리거
    float RightTrigger = NormalizeTrigger(CurrentGamepad.bRightTrigger);
    float PrevRightTrigger = NormalizeTrigger(PreviousGamepad.bRightTrigger);
    if (FMath::Abs(RightTrigger - PrevRightTrigger) > 0.01f)
    {
        OnXboxControllerAnalogInput(ControllerId, EXboxAnalog::RightTriggerAxis, RightTrigger);
    }
}

float FSlateAppMessageHandler::NormalizeThumbstick(SHORT Value, SHORT DeadZone) const
{
    // Promote everything to int to avoid overflow
    const int32 IntValue = static_cast<int32>(Value);
    const int32 IntDeadZone = static_cast<int32>(DeadZone);
    
    if (FMath::Abs(IntValue) < IntDeadZone)
    {
        return 0.0f;
    }

    if (IntValue > 0)
    {
        return static_cast<float>(IntValue - IntDeadZone) / static_cast<float>(32767 - IntDeadZone);
    }
    else // IntValue < 0
    {
        return static_cast<float>(IntValue + IntDeadZone) / static_cast<float>(32768 - IntDeadZone);
    }
}

float FSlateAppMessageHandler::NormalizeTrigger(BYTE Value) const
{
    if (Value < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
    {
        return 0.0f;
    }
    
    return static_cast<float>(Value - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / 
           static_cast<float>(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

void FSlateAppMessageHandler::OnKeyChar(const TCHAR Character, const bool IsRepeat)
{
    OnKeyCharDelegate.Broadcast(Character, IsRepeat);
}

void FSlateAppMessageHandler::OnKeyDown(const uint32 KeyCode, const uint32 CharacterCode, const bool IsRepeat)
{
    FInputKeyManager::Get();
    OnKeyDownDelegate.Broadcast(FKeyEvent{
        EKeys::Invalid, // TODO: 나중에 FInputKeyManager구현되면 바꾸기
        GetModifierKeys(),
        IsRepeat ? IE_Repeat : IE_Pressed,
        CharacterCode,
        KeyCode,
    });
}

void FSlateAppMessageHandler::OnKeyUp(const uint32 KeyCode, const uint32 CharacterCode, const bool IsRepeat)
{
    assert(!IsRepeat);  // KeyUp 이벤트에서 IsRepeat가 true일수가 없기 때문에

    OnKeyUpDelegate.Broadcast(FKeyEvent{
        EKeys::Invalid, // TODO: 나중에 FInputKeyManager구현되면 바꾸기
        GetModifierKeys(),
        IE_Released,
        CharacterCode,
        KeyCode,
    });
}

void FSlateAppMessageHandler::OnMouseDown(const EMouseButtons::Type Button, const FVector2D CursorPos)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    EKeys::Type EffectingButton = EKeys::Invalid;
    switch (Button)
    {
    case EMouseButtons::Left:
        EffectingButton = EKeys::LeftMouseButton;
        break;
    case EMouseButtons::Middle:
        EffectingButton = EKeys::MiddleMouseButton;
        break;
    case EMouseButtons::Right:
        EffectingButton = EKeys::RightMouseButton;
        break;
    case EMouseButtons::Thumb01:
        EffectingButton = EKeys::ThumbMouseButton;
        break;
    case EMouseButtons::Thumb02:
        EffectingButton = EKeys::ThumbMouseButton2;
        break;
    case EMouseButtons::Invalid:
        EffectingButton = EKeys::Invalid;
        break;
    }

    PressedMouseButtons.Add(EffectingButton);
    OnMouseDownDelegate.Broadcast(FPointerEvent{
        CursorPos,
        GetLastCursorPos(),
        0.0f,
        EffectingButton,
        PressedMouseButtons,
        GetModifierKeys(),
        IE_Pressed,
    });
}

void FSlateAppMessageHandler::OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    EKeys::Type EffectingButton = EKeys::Invalid;
    switch (Button)
    {
    case EMouseButtons::Left:
        EffectingButton = EKeys::LeftMouseButton;
        break;
    case EMouseButtons::Middle:
        EffectingButton = EKeys::MiddleMouseButton;
        break;
    case EMouseButtons::Right:
        EffectingButton = EKeys::RightMouseButton;
        break;
    case EMouseButtons::Thumb01:
        EffectingButton = EKeys::ThumbMouseButton;
        break;
    case EMouseButtons::Thumb02:
        EffectingButton = EKeys::ThumbMouseButton2;
        break;
    case EMouseButtons::Invalid:
        EffectingButton = EKeys::Invalid;
        break;
    }

    PressedMouseButtons.Remove(EffectingButton);
    OnMouseUpDelegate.Broadcast(FPointerEvent{
        CursorPos,
        GetLastCursorPos(),
        0.0f,
        EffectingButton,
        PressedMouseButtons,
        GetModifierKeys(),
        IE_Released,
    });
}

void FSlateAppMessageHandler::OnMouseDoubleClick(const EMouseButtons::Type Button, const FVector2D CursorPos)
{
    EKeys::Type EffectingButton = EKeys::Invalid;
    switch (Button)
    {
    case EMouseButtons::Left:
        EffectingButton = EKeys::LeftMouseButton;
        break;
    case EMouseButtons::Middle:
        EffectingButton = EKeys::MiddleMouseButton;
        break;
    case EMouseButtons::Right:
        EffectingButton = EKeys::RightMouseButton;
        break;
    case EMouseButtons::Thumb01:
        EffectingButton = EKeys::ThumbMouseButton;
        break;
    case EMouseButtons::Thumb02:
        EffectingButton = EKeys::ThumbMouseButton2;
        break;
    case EMouseButtons::Invalid:
        EffectingButton = EKeys::Invalid;
        break;
    }

    PressedMouseButtons.Add(EffectingButton);
    OnMouseDoubleClickDelegate.Broadcast(FPointerEvent{
        CursorPos,
        GetLastCursorPos(),
        0.0f,
        EffectingButton,
        PressedMouseButtons,
        GetModifierKeys(),
        IE_DoubleClick,
    });
}

void FSlateAppMessageHandler::OnMouseWheel(const float Delta, const FVector2D CursorPos)
{
    OnMouseWheelDelegate.Broadcast(FPointerEvent{
        CursorPos,
        GetLastCursorPos(),
        Delta,
        EKeys::MouseWheelAxis,
        PressedMouseButtons,
        GetModifierKeys(),
        IE_Axis,
    });
}

void FSlateAppMessageHandler::OnMouseMove()
{
    OnMouseMoveDelegate.Broadcast(FPointerEvent{
        GetCursorPos(),
        GetLastCursorPos(),
        0.0f,
        EKeys::Invalid,
        PressedMouseButtons,
        GetModifierKeys(),
        IE_Axis,
    });
}

void FSlateAppMessageHandler::UpdateXboxControllers(float DeltaTime)
{
    XboxControllerUpdateTimer += DeltaTime;
    
    // 60Hz로 업데이트
    if (XboxControllerUpdateTimer < XboxControllerUpdateInterval)
    {
        return;
    }
    
    XboxControllerUpdateTimer = 0.0f;

    for (uint32 ControllerId = 0; ControllerId < MaxControllers; ++ControllerId)
    {
        // 이전 상태 저장
        XboxPreviousStates[ControllerId] = XboxControllerStates[ControllerId];
        
        // 현재 상태 가져오기
        DWORD Result = XInputGetState(ControllerId, &XboxControllerStates[ControllerId]);
        
        bool bCurrentlyConnected = (Result == ERROR_SUCCESS);
        bool bWasConnected = XboxControllerConnected[ControllerId];
        
        // 연결 상태 변화 감지
        if (bCurrentlyConnected != bWasConnected)
        {
            XboxControllerConnected[ControllerId] = bCurrentlyConnected;
            
            if (bCurrentlyConnected)
            {
                OnXboxControllerConnected(ControllerId);
            }
            else
            {
                OnXboxControllerDisconnected(ControllerId);
                continue; // 연결이 끊어졌으면 입력 처리하지 않음
            }
        }
        
        if (!bCurrentlyConnected)
        {
            continue;
        }

        // 버튼 입력 처리
        ProcessXboxControllerButtons(ControllerId);
        ProcessXboxControllerAnalogInputs(ControllerId);
    }
}

void FSlateAppMessageHandler::OnXboxControllerButtonDown(uint32 ControllerId, EXboxButtons::Type Button, bool bIsRepeat)
{
    OnControllerButtonDownDelegate.Broadcast(FControllerButtonEvent{
        Button,
        GetModifierKeys(),
        bIsRepeat ? IE_Repeat : IE_Pressed,
        ControllerId,
        bIsRepeat
    });
}

void FSlateAppMessageHandler::OnXboxControllerButtonUp(uint32 ControllerId, EXboxButtons::Type Button)
{
    OnControllerButtonUpDelegate.Broadcast(FControllerButtonEvent{
        Button,
        GetModifierKeys(),
        IE_Released,
        ControllerId,
        false
    });
}

void FSlateAppMessageHandler::OnXboxControllerAnalogInput(uint32 ControllerId, EXboxAnalog::Type AnalogType, float Value)
{
    OnXboxControllerAnalogDelegate.Broadcast(FControllerAnalogEvent{
        AnalogType,
        GetModifierKeys(),
        IE_Axis,
        Value,
        ControllerId
    });
}

void FSlateAppMessageHandler::OnXboxControllerConnected(uint32 ControllerId)
{
    UE_LOG(ELogLevel::Display, "Xbox Controller %d connected", ControllerId);
    GEngine->ActiveWorld->ConnectedPlayer(ControllerId);
    OnXboxControllerConnectedDelegate.Broadcast(ControllerId);
}

void FSlateAppMessageHandler::OnXboxControllerDisconnected(uint32 ControllerId)
{
    UE_LOG(ELogLevel::Display, "Xbox Controller %d disconnected", ControllerId);
    
    // 진동 정지
    SetXboxControllerVibration(ControllerId, 0.0f, 0.0f);
    OnXboxControllerDisconnectedDelegate.Broadcast(ControllerId);
}

void FSlateAppMessageHandler::SetXboxControllerVibration(uint32 ControllerId, float LeftMotor, float RightMotor)
{
    if (ControllerId >= MaxControllers || !XboxControllerConnected[ControllerId])
    {
        return;
    }
    
    XINPUT_VIBRATION Vibration;
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));
    
    Vibration.wLeftMotorSpeed = static_cast<WORD>(FMath::Clamp(LeftMotor, 0.0f, 1.0f) * 65535);
    Vibration.wRightMotorSpeed = static_cast<WORD>(FMath::Clamp(RightMotor, 0.0f, 1.0f) * 65535);
    
    XInputSetState(ControllerId, &Vibration);
}

bool FSlateAppMessageHandler::IsXboxControllerConnected(uint32 ControllerId) const
{
    return ControllerId < MaxControllers && XboxControllerConnected[ControllerId];
}

void FSlateAppMessageHandler::OnRawMouseInput(const RAWMOUSE& RawMouseInput)
{
    // 눌린 버튼 상태 (PressedMouseButtons) 업데이트 및 EffectingButton 결정
    const USHORT ButtonFlags = LOWORD(RawMouseInput.ulButtons); // 하위 워드: 버튼 변경 플래그
    EKeys::Type EffectingButton = EKeys::Invalid;
    EInputEvent InputEventType = IE_None;
    float WheelDelta = 0.0f;

    // 눌린 버튼이 있는경우
    if (ButtonFlags)
    {
        if (PressedMouseButtons.IsEmpty())
        {
            // 커서가 화면 안에 있는지 검사
            RECT WindowRect;
            ::GetWindowRect(GEngineLoop.AppWnd, &WindowRect);
    
            POINT Pos;
            ::GetCursorPos(&Pos);
    
            if (!::PtInRect(&WindowRect, Pos))
            {
                return;
            }
        }

        // 마우스 왼쪽 버튼
        if (ButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
        {
            PressedMouseButtons.Add(EKeys::LeftMouseButton);
            EffectingButton = EKeys::LeftMouseButton;
            InputEventType = IE_Pressed;
        }
        else if (ButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
        {
            PressedMouseButtons.Remove(EKeys::LeftMouseButton);
            EffectingButton = EKeys::LeftMouseButton;
            InputEventType = IE_Released;
        }
        
        // 마우스 오른쪽 버튼
        else if (ButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
        {
            PressedMouseButtons.Add(EKeys::RightMouseButton);
            EffectingButton = EKeys::RightMouseButton;
            InputEventType = IE_Pressed;
        }
        else if (ButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
        {
            PressedMouseButtons.Remove(EKeys::RightMouseButton);
            EffectingButton = EKeys::RightMouseButton;
            InputEventType = IE_Released;
        }
        
        // 마우스 가운데 버튼
        else if (ButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
        {
            PressedMouseButtons.Add(EKeys::MiddleMouseButton);
            EffectingButton = EKeys::MiddleMouseButton;
            InputEventType = IE_Pressed;
        }
        else if (ButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
        {
            PressedMouseButtons.Remove(EKeys::MiddleMouseButton);
            EffectingButton = EKeys::MiddleMouseButton;
            InputEventType = IE_Released;
        }
        
        // 마우스 엄지버튼 1
        else if (ButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
        {
            PressedMouseButtons.Add(EKeys::ThumbMouseButton);
            EffectingButton = EKeys::ThumbMouseButton;
            InputEventType = IE_Pressed;
        }
        else if (ButtonFlags & RI_MOUSE_BUTTON_4_UP)
        {
            PressedMouseButtons.Remove(EKeys::ThumbMouseButton);
            EffectingButton = EKeys::ThumbMouseButton;
            InputEventType = IE_Released;
        }
        
        // 마우스 엄지버튼 2
        else if (ButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
        {
            PressedMouseButtons.Add(EKeys::ThumbMouseButton2);
            EffectingButton = EKeys::ThumbMouseButton2;
            InputEventType = IE_Pressed;
        }
        else if (ButtonFlags & RI_MOUSE_BUTTON_5_UP)
        {
            PressedMouseButtons.Remove(EKeys::ThumbMouseButton2);
            EffectingButton = EKeys::ThumbMouseButton2;
            InputEventType = IE_Released;
        }

        // 마우스 휠
        else if (ButtonFlags & RI_MOUSE_WHEEL)
        {
            const SHORT WheelData = static_cast<SHORT>(HIWORD(RawMouseInput.ulButtons));
            WheelDelta = static_cast<float>(WheelData) / static_cast<float>(WHEEL_DELTA);
            EffectingButton = EKeys::MouseWheelAxis;
            InputEventType = IE_Axis;
        }
        else if (ButtonFlags & RI_MOUSE_WHEEL)
        {
            // TODO: 추후에 수평 휠 처리 (RI_MOUSE_HWHEEL) 가 필요하면 추가
        }

        OnRawMouseInputDelegate.Broadcast(FPointerEvent{
            GetCursorPos(),
            GetLastCursorPos(),
            FVector2D::ZeroVector,
            WheelDelta,
            EffectingButton,
            PressedMouseButtons,
            GetModifierKeys(),
            InputEventType,
        });
    }

    // 버튼과 관련없는 이벤트
    else [[likely]]
    {
        // 그냥 마우스를 움직일 때 발생
        if (RawMouseInput.usFlags == MOUSE_MOVE_RELATIVE) [[likely]]
        {
            EffectingButton = EKeys::Invalid;
            InputEventType = IE_Axis;
        }
        else if (RawMouseInput.usFlags & MOUSE_MOVE_ABSOLUTE)
        {
            // 태블릿, 터치스크린, 고급 트랙패드 같은 장치들에서 이벤트 발생
            // TODO: 언젠가 구?현 하기
            UE_LOG(ELogLevel::Warning, "Absolute mouse movement detected (currently not fully supported).");
        }

        OnRawMouseInputDelegate.Broadcast(FPointerEvent{
            GetCursorPos(),
            GetLastCursorPos(),
            FVector2D{
                static_cast<float>(RawMouseInput.lLastX),
                static_cast<float>(RawMouseInput.lLastY)
            },
            WheelDelta,
            EffectingButton,
            PressedMouseButtons,
            GetModifierKeys(),
            InputEventType,
        });
    }
}

void FSlateAppMessageHandler::OnRawKeyboardInput(const RAWKEYBOARD& RawKeyboardInput)
{
    // 입력 이벤트 타입 설정
    const EInputEvent InputEventType = (RawKeyboardInput.Flags & RI_KEY_BREAK) ? IE_Released : IE_Pressed;

    OnRawKeyboardInputDelegate.Broadcast(FKeyEvent{
        EKeys::Invalid,  // TODO: 나중에 FInputKeyManager구현되면 바꾸기
        GetModifierKeys(),
        InputEventType,
        0, // RawInput에서 Char를 얻기 어렵기 때문에
        RawKeyboardInput.VKey
    });
}

void FSlateAppMessageHandler::OnPIEModeStart()
{
    OnPIEModeStartDelegate.Broadcast();
}

void FSlateAppMessageHandler::OnPIEModeEnd()
{
    OnPIEModeEndDelegate.Broadcast();
}

void FSlateAppMessageHandler::UpdateCursorPosition(const FVector2D& NewPos)
{
    PreviousPosition = CurrentPosition;
    CurrentPosition = NewPos;
}

FVector2D FSlateAppMessageHandler::GetCursorPos() const
{
    return CurrentPosition;
}

FVector2D FSlateAppMessageHandler::GetLastCursorPos() const
{
    return PreviousPosition;
}

FModifierKeysState FSlateAppMessageHandler::GetModifierKeys() const
{
    return FModifierKeysState{
        ModifierKeyState[EModifierKey::LeftShift],
        ModifierKeyState[EModifierKey::RightShift],
        ModifierKeyState[EModifierKey::LeftControl],
        ModifierKeyState[EModifierKey::RightControl],
        ModifierKeyState[EModifierKey::LeftAlt],
        ModifierKeyState[EModifierKey::RightAlt],
        ModifierKeyState[EModifierKey::LeftWin],
        ModifierKeyState[EModifierKey::RightWin],
        ModifierKeyState[EModifierKey::CapsLock]
    };
}
