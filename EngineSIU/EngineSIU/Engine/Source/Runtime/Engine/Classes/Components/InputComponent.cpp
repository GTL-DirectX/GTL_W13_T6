#include "InputComponent.h"

#include <xinput.h>

#include "WindowsCursor.h"

void UInputComponent::CreateXInputController(int PlayerIndex)
{
    ControllerID = PlayerIndex;
}

void UInputComponent::ProcessInput(float DeltaTime)
{
    if (PressedKeys.Contains(EKeys::W))
    {
        KeyBindDelegate[FString("W")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::A))
    {
        KeyBindDelegate[FString("A")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::S))
    {
        KeyBindDelegate[FString("S")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::D))
    {
        KeyBindDelegate[FString("D")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::Q))
    {
        KeyBindDelegate[FString("Q")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::E))
    {
        KeyBindDelegate[FString("E")].Broadcast(DeltaTime);
    }

    ////////////////// Pad Input ////////////////
    ProcessControllerButton(DeltaTime);
    ProcessControllerAnalog(DeltaTime);
}

void UInputComponent::ProcessControllerButton(float DeltaTime)
{
    // Xbox 컨트롤러 버튼 체크 if 분기문들

    // 기본 액션 버튼들
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_A))
    {
        UE_LOG(ELogLevel::Display, "Pressed A Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_A].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_B))
    {
        UE_LOG(ELogLevel::Display, "Pressed B Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_B].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_X))
    {
        UE_LOG(ELogLevel::Display, "Pressed X Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_X].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_Y))
    {
        UE_LOG(ELogLevel::Display, "Pressed Y Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_Y].Broadcast(DeltaTime);
    }

    // 범퍼 버튼들 (어깨 버튼)
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_LEFT_SHOULDER))
    {
        UE_LOG(ELogLevel::Display, "Pressed Left Bumper (LB)");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_LEFT_SHOULDER].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_RIGHT_SHOULDER))
    {
        UE_LOG(ELogLevel::Display, "Pressed Right Bumper (RB)");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_RIGHT_SHOULDER].Broadcast(DeltaTime);
    }

    // 시스템 버튼들
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_BACK))
    {
        UE_LOG(ELogLevel::Display, "Pressed Back Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_BACK].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_START))
    {
        UE_LOG(ELogLevel::Display, "Pressed Start Button");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_START].Broadcast(DeltaTime);
    }

    // 스틱 클릭 버튼들
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_LEFT_THUMB))
    {
        UE_LOG(ELogLevel::Display, "Pressed Left Thumbstick Click");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_LEFT_THUMB].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_RIGHT_THUMB))
    {
        UE_LOG(ELogLevel::Display, "Pressed Right Thumbstick Click");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_RIGHT_THUMB].Broadcast(DeltaTime);
    }

    // 방향패드 (D-Pad) 버튼들
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_DPAD_UP))
    {
        UE_LOG(ELogLevel::Display, "Pressed D-Pad Up");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_DPAD_UP].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_DPAD_DOWN))
    {
        UE_LOG(ELogLevel::Display, "Pressed D-Pad Down");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_DPAD_DOWN].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_DPAD_LEFT))
    {
        UE_LOG(ELogLevel::Display, "Pressed D-Pad Left");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_DPAD_LEFT].Broadcast(DeltaTime);
    }
    if (PressedControllerButtons.Contains(XINPUT_GAMEPAD_DPAD_RIGHT))
    {
        UE_LOG(ELogLevel::Display, "Pressed D-Pad Right");
        ControllerButtonBindDelegate[XINPUT_GAMEPAD_DPAD_RIGHT].Broadcast(DeltaTime);
    }
}

void UInputComponent::ProcessControllerAnalog(float DeltaTime)
{
    // 참고: 아날로그 스틱 값들은 별도 처리
    // 왼쪽 스틱
    float LeftStickX = CurrentAnalogValues[EXboxAnalog::LeftStickX];
    float LeftStickY = CurrentAnalogValues[EXboxAnalog::LeftStickY];
    if (FMath::Abs(LeftStickX) > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Left Stick X: %f", LeftStickX);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::LeftStickX].Broadcast(DeltaTime);
    }
    if (FMath::Abs(LeftStickY) > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Left Stick Y: %f", LeftStickY);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::LeftStickY].Broadcast(DeltaTime);
    }

    // 오른쪽 스틱
    float RightStickX = CurrentAnalogValues[EXboxAnalog::RightStickX];
    float RightStickY = CurrentAnalogValues[EXboxAnalog::RightStickY];
    if (FMath::Abs(RightStickX) > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Right Stick X: %f", RightStickX);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::RightStickX].Broadcast(DeltaTime);
    }
    if (FMath::Abs(RightStickY) > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Right Stick Y: %f", RightStickY);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::RightStickY].Broadcast(DeltaTime);
    }

    // 트리거 아날로그 값
    float LeftTriggerValue =  CurrentAnalogValues[EXboxAnalog::LeftTriggerAxis];
    float RightTriggerValue = CurrentAnalogValues[EXboxAnalog::RightTriggerAxis];
    if (LeftTriggerValue > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Left Trigger Analog: %f", LeftTriggerValue);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::LeftTriggerAxis].Broadcast(DeltaTime);
    }
    if (RightTriggerValue > 0.1f)
    {
        UE_LOG(ELogLevel::Display, "Right Trigger Analog: %f", RightTriggerValue);
        ControllerAnalogBindDelegate[EXboxAnalog::Type::RightTriggerAxis].Broadcast(DeltaTime);
    }
}

void UInputComponent::SetPossess()
{
    BindInputDelegate();
    

    //TODO: Possess일때 기존에 있던거 다시 넣어줘야할수도
}

void UInputComponent::BindInputDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    BindKeyDownDelegateHandles.Add(Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            InputKey(InKeyEvent);
        }));

    BindKeyUpDelegateHandles.Add(Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            InputKey(InKeyEvent);
        }));
    BindMouseMoveDelegateHandles.Add(Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            const FVector2D& Delta = InMouseEvent.GetCursorDelta();

            if (PressedKeys.Contains(EKeys::RightMouseButton))
            {
                if (MouseBindDelegate.Contains("Turn"))
                {
                    MouseBindDelegate["Turn"].Broadcast(Delta.X);
                }

                if (MouseBindDelegate.Contains("LookUp"))
                {
                    MouseBindDelegate["LookUp"].Broadcast(Delta.Y);
                }
            }
        }));

    BindMouseDownDelegateHandles.Add(Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent) {
        InputMouse(InMouseEvent);
        }));
    BindMouseUpDelegateHandles.Add(Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent) {
        InputMouse(InMouseEvent);
        }));

    /////////// Pad Input /////////////
    BindControllerDownDelegateHandles.Add(Handler->OnControllerButtonDownDelegate.AddLambda([this](const FControllerButtonEvent& InControllerEvent)
        {
            InputControllerButton(InControllerEvent);
        }));
    
    BindControllerUpDelegateHandles.Add(Handler->OnControllerButtonUpDelegate.AddLambda([this](const FControllerButtonEvent& InControllerEvent)
    {
        InputControllerButton(InControllerEvent);
    }));
    
    // 아날로그 입력 이벤트 바인딩 (새로 추가)
    BindControllerAnalogDelegateHandles.Add(Handler->OnXboxControllerAnalogDelegate.AddLambda([this](const FControllerAnalogEvent& InAnalogEvent)
    {
        InputControllerAnalog(InAnalogEvent);
    }));
    
}

void UInputComponent::UnPossess()
{
    ClearBindDelegate();
}

void UInputComponent::ClearBindDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    for (FDelegateHandle DelegateHandle : BindKeyDownDelegateHandles)
    {
        Handler->OnKeyDownDelegate.Remove(DelegateHandle);
    }

    for (FDelegateHandle DelegateHandle : BindKeyUpDelegateHandles)
    {
        Handler->OnKeyUpDelegate.Remove(DelegateHandle);
    }

    for (FDelegateHandle DelegateHandle : BindMouseMoveDelegateHandles)
    {
        Handler->OnMouseMoveDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindMouseDownDelegateHandles)
    {
        Handler->OnMouseDownDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindMouseUpDelegateHandles)
    {
        Handler->OnMouseUpDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindControllerDownDelegateHandles)
    {
        Handler->OnControllerButtonDownDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindControllerUpDelegateHandles)
    {
        Handler->OnControllerButtonUpDelegate.Remove(DelegateHandle);
    }

    BindKeyDownDelegateHandles.Empty();
    BindKeyUpDelegateHandles.Empty();
    BindMouseMoveDelegateHandles.Empty();
    BindMouseDownDelegateHandles.Empty();
    BindMouseUpDelegateHandles.Empty();
    BindControllerDownDelegateHandles.Empty();
    BindControllerUpDelegateHandles.Empty();
}

void UInputComponent::InputKey(const FKeyEvent& InKeyEvent)
{
    // 일반적인 단일 키 이벤트
    switch (InKeyEvent.GetCharacter())
    {
    case 'W':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::W);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::W);
        }
        break;
    }
    case 'A':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::A);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::A);
        }
        break;
    }
    case 'S':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::S);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::S);
        }
        break;
    }
    case 'D':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::D);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::D);
        }
        break;
    }
    case 'Q':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::Q);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::Q);
        }
        break;
    }
    case 'E':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::E);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::E);
        }
        break;
    }
    default:
        break;
    }
}

void UInputComponent::InputMouse(const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (InMouseEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::RightMouseButton);
            FWindowsCursor::SetShowMouseCursor(false);
            MousePinPosition = InMouseEvent.GetScreenSpacePosition();
        }
        else if (InMouseEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::RightMouseButton);
            FWindowsCursor::SetShowMouseCursor(true);
            FWindowsCursor::SetPosition(
                static_cast<int32>(MousePinPosition.X),
                static_cast<int32>(MousePinPosition.Y)
            );
        }
    }
}

void UInputComponent::InputControllerButton(const FControllerButtonEvent& InButtonEvent)
{
    // 플레이어 ID 필터링 (필요시)
    if (InButtonEvent.GetControllerId() != ControllerID) // Player 0만 처리
    {
        return;
    }
    
    EXboxButtons::Type Button = InButtonEvent.GetButton();
    EInputEvent InputEvent = InButtonEvent.GetInputEvent();
    
    // Xbox 버튼을 WORD 타입으로 변환
    WORD ButtonMask = 0;
    
    switch (Button)
    {
    case EXboxButtons::A:
        ButtonMask = XINPUT_GAMEPAD_A;
        break;
    case EXboxButtons::B:
        ButtonMask = XINPUT_GAMEPAD_B;
        break;
    case EXboxButtons::X:
        ButtonMask = XINPUT_GAMEPAD_X;
        break;
    case EXboxButtons::Y:
        ButtonMask = XINPUT_GAMEPAD_Y;
        break;
    case EXboxButtons::LeftBumper:
        ButtonMask = XINPUT_GAMEPAD_LEFT_SHOULDER;
        break;
    case EXboxButtons::RightBumper:
        ButtonMask = XINPUT_GAMEPAD_RIGHT_SHOULDER;
        break;
    case EXboxButtons::Back:
        ButtonMask = XINPUT_GAMEPAD_BACK;
        break;
    case EXboxButtons::Start:
        ButtonMask = XINPUT_GAMEPAD_START;
        break;
    case EXboxButtons::LeftThumbstick:
        ButtonMask = XINPUT_GAMEPAD_LEFT_THUMB;
        break;
    case EXboxButtons::RightThumbstick:
        ButtonMask = XINPUT_GAMEPAD_RIGHT_THUMB;
        break;
    case EXboxButtons::DPadUp:
        ButtonMask = XINPUT_GAMEPAD_DPAD_UP;
        break;
    case EXboxButtons::DPadDown:
        ButtonMask = XINPUT_GAMEPAD_DPAD_DOWN;
        break;
    case EXboxButtons::DPadLeft:
        ButtonMask = XINPUT_GAMEPAD_DPAD_LEFT;
        break;
    case EXboxButtons::DPadRight:
        ButtonMask = XINPUT_GAMEPAD_DPAD_RIGHT;
        break;
    case EXboxButtons::LeftTrigger:
        ButtonMask = 0x8000; // 가상 트리거 버튼 마스크
        break;
    case EXboxButtons::RightTrigger:
        ButtonMask = 0x4000; // 가상 트리거 버튼 마스크
        break;
    default:
        UE_LOG(ELogLevel::Warning, "Unknown controller button: %d", static_cast<int32>(Button));
        return;
    }
    
    // 버튼 상태 업데이트
    if (InputEvent == IE_Pressed)
    {
        PressedControllerButtons.Add(ButtonMask);
    }
    else if (InputEvent == IE_Released)
    {
        PressedControllerButtons.Remove(ButtonMask);
    }
}

void UInputComponent::InputControllerAnalog(const FControllerAnalogEvent& InAnalogEvent)
{
    // 플레이어 ID 필터링 (필요시)
    if (InAnalogEvent.GetControllerId() != ControllerID) // Player 0만 처리
    {
        return;
    }
    
    EXboxAnalog::Type AnalogType = InAnalogEvent.GetAnalogType();
    float AnalogValue = InAnalogEvent.GetAnalogValue();
    
    // 현재 아날로그 값 저장 (필요시 이전 값과 비교 가능)
    float* PreviousValue = CurrentAnalogValues.Find(AnalogType); // TODO: 파이 모드 진입 후 스틱을 가장 먼저 조작하면 터짐
    if (PreviousValue == nullptr)
    {
        int temp = 0;
    }
    float PrevValue = PreviousValue ? *PreviousValue : 0.0f;
    CurrentAnalogValues.Add(AnalogType, AnalogValue);
    
    // 임계값 확인 (작은 변화는 무시)
    const float DeadZone = 0.1f;
    const float ChangeThreshold = 0.01f; // 변화량 임계값
    
    bool bSignificantChange = FMath::Abs(AnalogValue - PrevValue) > ChangeThreshold;
    bool bAboveDeadZone = FMath::Abs(AnalogValue) > DeadZone;
    
    if (!bSignificantChange && !bAboveDeadZone)
    {
        return; // 의미있는 변화가 없으면 처리하지 않음
    }
    
    // 아날로그 타입별 처리
    switch (AnalogType)
    {
    case EXboxAnalog::LeftStickX:
        {
            if (FMath::Abs(AnalogValue) > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Left Stick X: %f", AnalogValue);
                // 좌우 이동 처리
                // 예: Character->AddMovementInput(RightVector, AnalogValue);
            }
        }
        break;
        
    case EXboxAnalog::LeftStickY:
        {
            if (FMath::Abs(AnalogValue) > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Left Stick Y: %f", AnalogValue);
                // 전후 이동 처리
                // 예: Character->AddMovementInput(ForwardVector, AnalogValue);
            }
        }
        break;
        
    case EXboxAnalog::RightStickX:
        {
            if (FMath::Abs(AnalogValue) > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Right Stick X: %f", AnalogValue);
                // 카메라 좌우 회전 처리
                // 예: PlayerController->AddYawInput(AnalogValue * CameraSensitivity);
            }
        }
        break;
        
    case EXboxAnalog::RightStickY:
        {
            if (FMath::Abs(AnalogValue) > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Right Stick Y: %f", AnalogValue);
                // 카메라 상하 회전 처리
                // 예: PlayerController->AddPitchInput(AnalogValue * CameraSensitivity);
            }
        }
        break;
        
    case EXboxAnalog::LeftTriggerAxis:
        {
            if (AnalogValue > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Left Trigger Analog: %f", AnalogValue);
                // 왼쪽 트리거 처리 (예: 브레이크, 조준)
                // 예: Vehicle->Brake(AnalogValue);
            }
        }
        break;
        
    case EXboxAnalog::RightTriggerAxis:
        {
            if (AnalogValue > DeadZone)
            {
                UE_LOG(ELogLevel::Display, "Right Trigger Analog: %f", AnalogValue);
                // 오른쪽 트리거 처리 (예: 가속, 사격)
                // 예: Vehicle->Accelerate(AnalogValue);
            }
        }
        break;
        
    default:
        UE_LOG(ELogLevel::Warning, "Unknown analog input type: %d", static_cast<int32>(AnalogType));
        break;
    }
}

void UInputComponent::BindAction(const FString& Key, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    KeyBindDelegate[Key].AddLambda([this, Callback](float DeltaTime)
        {
            Callback(DeltaTime);
        });
}

void UInputComponent::BindAxis(const FString& Axis, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    MouseBindDelegate[Axis].AddLambda([this, Callback](float DeltaTime)
        {
            Callback(DeltaTime);
        });
}

void UInputComponent::BindControllerButton(const WORD& Button, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    ControllerButtonBindDelegate[Button].AddLambda([this, Callback](float DeltaTime)
        {
            Callback(DeltaTime);
        });
}

void UInputComponent::BindControllerAnalog(const EXboxAnalog::Type Axis, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    ControllerAnalogBindDelegate[Axis].AddLambda([this, Callback, Axis](float DeltaTime)
        {
            float AxisValue = CurrentAnalogValues[Axis];
            Callback(DeltaTime * AxisValue);
        });
}

void UInputComponent::BindControllerConnected(int ArrIndex, const std::function<void(int)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    ControllerAnalogBindDelegate[ArrIndex].AddLambda([this, Callback](int Index)
        {
            Callback(Index);
        });
}

void UInputComponent::BindControllerDisconnected(int ArrIndex, const std::function<void(int)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    ControllerAnalogBindDelegate[ArrIndex].AddLambda([this, Callback](int Index)
        {
            Callback(Index);
        });
}
