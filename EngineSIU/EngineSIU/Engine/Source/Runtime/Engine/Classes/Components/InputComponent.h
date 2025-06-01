#pragma once

#include "Core/Container/Map.h"
#include "Delegates/DelegateCombination.h"
#include "Runtime/InputCore/InputCoreTypes.h"
#include "Components/ActorComponent.h"

class FXInputController;
DECLARE_MULTICAST_DELEGATE_OneParam(FOneFloatDelegate, const float&)
DECLARE_MULTICAST_DELEGATE_OneParam(FAxisDelegate, const float&)

class UInputComponent : public UActorComponent
{
    DECLARE_CLASS(UInputComponent, UActorComponent)


public:
    UInputComponent() = default;
    virtual ~UInputComponent() override = default;

    void CreateXInputController(int PlayerIndex);
    
    void ProcessInput(float DeltaTime);
    void ProcessControllerButton(float DeltaTime);
    void ProcessControllerAnalog(float DeltaTime);
    
    void SetPossess();
    void UnPossess();
    
    void BindAction(const FString& Key, const std::function<void(float)>& Callback);
    void BindAxis(const FString& Axis, const std::function<void(float)>& Callback);
    void BindControllerButton(const WORD& Button, const std::function<void(float)>& Callback);
    void BindControllerAnalog(const EXboxAnalog::Type Axis, const std::function<void(float)>& Callback);
    void BindControllerConnected(int ArrIndex, const std::function<void(int)>& Callback);
    void BindControllerDisconnected(int ArrIndex, const std::function<void(int)>& Callback);
    
    void BindInputDelegate();
    void ClearBindDelegate();
    
    // Possess가 풀렸다가 다시 왔을때 원래 바인딩 돼있던 애들 일괄적으로 다시 바인딩해줘야할수도 있음.
    void InputKey(const FKeyEvent& InKeyEvent);
    void InputMouse(const FPointerEvent& InMouseEvent);
    void InputControllerButton(const FControllerButtonEvent& InButtonEvent);
    void InputControllerAnalog(const FControllerAnalogEvent& InAnalogEvent);
    
private:
    TMap<FString, FOneFloatDelegate> KeyBindDelegate;
    TMap<FString, FAxisDelegate> MouseBindDelegate;
    TArray<FDelegateHandle> BindKeyDownDelegateHandles;
    TArray<FDelegateHandle> BindKeyUpDelegateHandles;
    TArray<FDelegateHandle> BindMouseMoveDelegateHandles;
    TArray<FDelegateHandle> BindMouseDownDelegateHandles;
    TArray<FDelegateHandle> BindMouseUpDelegateHandles;
    TSet<EKeys::Type> PressedKeys;

    FVector2D MousePinPosition;

////////////////// Pad Input //////////////////////
    int ControllerID = -1;

    TMap<WORD, FOneFloatDelegate> ControllerButtonBindDelegate;
    TMap<WORD, FAxisDelegate> ControllerAnalogBindDelegate;
    
    TArray<FDelegateHandle> BindControllerDownDelegateHandles;
    TArray<FDelegateHandle> BindControllerUpDelegateHandles;
    TArray<FDelegateHandle> BindControllerAnalogDelegateHandles;

    TSet<WORD> PressedControllerButtons;
    TMap<EXboxAnalog::Type, float> CurrentAnalogValues;
};
