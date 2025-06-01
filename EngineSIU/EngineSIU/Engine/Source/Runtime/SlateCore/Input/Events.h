#pragma once
#include "Container/Set.h"
#include "InputCore/InputCoreTypes.h"
#include "Math/Vector.h"


class FKeySet : public TSet<EKeys::Type>
{
public:
    FKeySet(EKeys::Type Key)
    {
        Add(Key);
    }

    static const FKeySet EmptySet;
};

/**
 * 입력 이벤트 (Base)
 */
struct FInputEvent
{
    FInputEvent()
        : ModifierKeys(FModifierKeysState{})
        , InputEvent(IE_None)
    {
    }

    FInputEvent(const FModifierKeysState& InModifierKeys, EInputEvent InInputEvent)
        : ModifierKeys(InModifierKeys)
        , InputEvent(InInputEvent)
    {
    }

    FInputEvent(const FInputEvent&) = default;
    FInputEvent& operator=(const FInputEvent&) = default;
    FInputEvent(FInputEvent&&) = default;
    FInputEvent& operator=(FInputEvent&&) = default;

    virtual ~FInputEvent() = default;

public:
    /** 마우스 이벤트인지 여부 */
    virtual bool IsPointerEvent() const { return false; }

    /** 키보드 이벤트인지 여부 */
    virtual bool IsKeyEvent() const { return false; }

    /* 컨트롤러 이벤트인지 여부 */
    virtual bool IsControllerEvent() const { return false; }

public:
    /** 이 키 이벤트가 자동 반복 입력인지 여부를 반환합니다. */
    bool IsRepeat() const
    {
        return InputEvent == IE_Repeat;
    }

    /** Shift 키 중 하나라도 눌려 있었는지 여부를 반환합니다. */
    bool IsShiftDown() const
    {
        return ModifierKeys.IsShiftDown();
    }

    /** 왼쪽 Shift 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsLeftShiftDown() const
    {
        return ModifierKeys.IsLeftShiftDown();
    }

    /** 오른쪽 Shift 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsRightShiftDown() const
    {
        return ModifierKeys.IsRightShiftDown();
    }

    /** Control 키 중 하나라도 눌려 있었는지 여부를 반환합니다. */
    bool IsControlDown() const
    {
        return ModifierKeys.IsControlDown();
    }

    /** 왼쪽 Control 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsLeftControlDown() const
    {
        return ModifierKeys.IsLeftControlDown();
    }

    /** 오른쪽 Control 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsRightControlDown() const
    {
        return ModifierKeys.IsRightControlDown();
    }

    /** Alt 키 중 하나라도 눌려 있었는지 여부를 반환합니다. */
    bool IsAltDown() const
    {
        return ModifierKeys.IsAltDown();
    }

    /** 왼쪽 Alt 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsLeftAltDown() const
    {
        return ModifierKeys.IsLeftAltDown();
    }

    /** 오른쪽 Alt 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsRightAltDown() const
    {
        return ModifierKeys.IsRightAltDown();
    }

    /** Windows 키 중 하나라도 눌려 있었는지 여부를 반환합니다. */
    bool IsWindowsKeyDown() const
    {
        return ModifierKeys.IsWindowsKeyDown();
    }

    /** 왼쪽 Windows 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsLeftWinDown() const
    {
        return ModifierKeys.IsLeftWinDown();
    }

    /** 오른쪽 Windows 키가 눌려 있었는지 여부를 반환합니다. */
    bool IsRightWinDown() const
    {
        return ModifierKeys.IsRightWinDown();
    }

    /** Caps Lock 키가 활성화 상태인지 여부를 반환합니다. */
    bool AreCapsLocked() const
    {
        return ModifierKeys.AreCapsLocked();
    }

    const FModifierKeysState& GetModifierKeys() const
    {
        return ModifierKeys;
    }

    EInputEvent GetInputEvent() const
    {
        return InputEvent;
    }

protected:
    FModifierKeysState ModifierKeys;
    EInputEvent InputEvent;
};

/**
 * 키보드 이벤트
 */
struct FKeyEvent : public FInputEvent
{
    FKeyEvent()
        : FInputEvent(FModifierKeysState{}, IE_None)
        , Key(EKeys::Invalid)
        , CharacterCode(0)
        , KeyCode(0)
    {
    }

    FKeyEvent(
        const EKeys::Type InKey,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent,
        const uint32 InCharacterCode,
        const uint32 InKeyCode
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , Key(InKey)
        , CharacterCode(InCharacterCode)
        , KeyCode(InKeyCode)
    {
    }

public:
    EKeys::Type GetKey() const
    {
        return Key;
    }

    uint32 GetCharacter() const
    {
        return CharacterCode;
    }

    uint32 GetKeyCode() const
    {
        return KeyCode;
    }

    virtual bool IsKeyEvent() const override { return true; }

private:
    EKeys::Type Key;
    uint32 CharacterCode;
    uint32 KeyCode;
};

/**
 * 마우스 이벤트
 */
struct FPointerEvent : public FInputEvent
{
    FPointerEvent()
        : FInputEvent(FModifierKeysState{}, IE_None)
        , ScreenSpacePosition(FVector2D::ZeroVector)
        , LastScreenSpacePosition(FVector2D::ZeroVector)
        , CursorDelta(FVector2D::ZeroVector)
        , PressedButtons(&FKeySet::EmptySet)
        , EffectingButton(EKeys::Invalid)
        , WheelDelta(0.0f)
    {
    }

    FPointerEvent(
        FVector2D InScreenSpacePosition,
        FVector2D InLastScreenSpacePosition,
        FVector2D InCursorDelta,
        float InWheelDelta,
        EKeys::Type InEffectingButton,
        const TSet<EKeys::Type>& InPressedButtons,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , ScreenSpacePosition(InScreenSpacePosition)
        , LastScreenSpacePosition(InLastScreenSpacePosition)
        , CursorDelta(InCursorDelta)
        , PressedButtons(&InPressedButtons)
        , EffectingButton(InEffectingButton)
        , WheelDelta(InWheelDelta)
    {
    }

    FPointerEvent(
        FVector2D InScreenSpacePosition,
        FVector2D InLastScreenSpacePosition,
        float InWheelDelta,
        EKeys::Type InEffectingButton,
        const TSet<EKeys::Type>& InPressedButtons,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , ScreenSpacePosition(InScreenSpacePosition)
        , LastScreenSpacePosition(InLastScreenSpacePosition)
        , CursorDelta(InScreenSpacePosition - InLastScreenSpacePosition)
        , PressedButtons(&InPressedButtons)
        , EffectingButton(InEffectingButton)
        , WheelDelta(InWheelDelta)
    {
    }

    /** 현재 마우스 포인터의 위치를 가져옵니다. */
    const FVector2D& GetScreenSpacePosition() const { return ScreenSpacePosition; }

    /** 한 프레임 전의 마우스 포인터의 위치를 가져옵니다. */
    const FVector2D& GetLastScreenSpacePosition() const { return LastScreenSpacePosition; }

    /** 마우스 포인터의 이동량을 가져옵니다. */
    const FVector2D& GetCursorDelta() const { return CursorDelta; }

    /** Button이 눌려 있는지 검사합니다. */
    bool IsMouseButtonDown(EKeys::Type Button) const { return PressedButtons->Contains(Button); }

    /** 이벤트가 시작된 버튼을 가져옵니다. */
    EKeys::Type GetEffectingButton() const { return EffectingButton; }

    /** 마우스 휠의 이동량을 가져옵니다. */
    float GetWheelDelta() const { return WheelDelta; }

    virtual bool IsPointerEvent() const override { return true; }

private:
    FVector2D ScreenSpacePosition;
    FVector2D LastScreenSpacePosition;
    FVector2D CursorDelta;
    const TSet<EKeys::Type>* PressedButtons;
    EKeys::Type EffectingButton;
    float WheelDelta;
};

// Xbox 컨트롤러 버튼 이벤트 구조체
struct FControllerButtonEvent : public FInputEvent
{
    FControllerButtonEvent()
        : FInputEvent(FModifierKeysState{}, IE_None)
        , Button(EXboxButtons::Invalid)
        , ControllerId(0)
        , bIsRepeat(false)
    {
    }

    FControllerButtonEvent(
        const EXboxButtons::Type InButton,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent,
        const uint32 InControllerId,
        const bool bInIsRepeat = false
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , Button(InButton)
        , ControllerId(InControllerId)
        , bIsRepeat(bInIsRepeat)
    {
    }

public:
    EXboxButtons::Type GetButton() const
    {
        return Button;
    }

    uint32 GetControllerId() const
    {
        return ControllerId;
    }

    bool IsRepeat() const
    {
        return bIsRepeat;
    }

    virtual bool IsControllerEvent() const override { return true; }

private:
    EXboxButtons::Type Button;
    uint32 ControllerId;
    bool bIsRepeat;
};

// Xbox 컨트롤러 아날로그 이벤트 구조체
struct FControllerAnalogEvent : public FInputEvent
{
    FControllerAnalogEvent()
        : FInputEvent(FModifierKeysState{}, IE_None)
        , AnalogType(EXboxAnalog::Invalid)
        , AnalogValue(0.0f)
        , ControllerId(0)
        , DeltaTime(0.0f)
    {
    }

    FControllerAnalogEvent(
        const EXboxAnalog::Type InAnalogType,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent,
        const float InAnalogValue,
        const uint32 InControllerId,
        const float InDeltaTime = 0.0f
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , AnalogType(InAnalogType)
        , AnalogValue(InAnalogValue)
        , ControllerId(InControllerId)
        , DeltaTime(InDeltaTime)
    {
    }

public:
    EXboxAnalog::Type GetAnalogType() const
    {
        return AnalogType;
    }

    float GetAnalogValue() const
    {
        return AnalogValue;
    }

    uint32 GetControllerId() const
    {
        return ControllerId;
    }

    float GetDeltaTime() const
    {
        return DeltaTime;
    }

    virtual bool IsKeyEvent() const override { return false; }
    virtual bool IsControllerEvent() const override { return true; }
    virtual bool IsAnalogEvent() const { return true; }

private:
    EXboxAnalog::Type AnalogType;
    float AnalogValue;
    uint32 ControllerId;
    float DeltaTime;
};

// 통합 Xbox 컨트롤러 이벤트 구조체
struct FXboxControllerEvent : public FInputEvent
{
    FXboxControllerEvent()
        : FInputEvent(FModifierKeysState{}, IE_None)
        , Button(EXboxButtons::Invalid)
        , AnalogType(EXboxAnalog::Invalid)
        , AnalogValue(0.0f)
        , ControllerId(0)
        , bIsRepeat(false)
        , bIsAnalogEvent(false)
    {
    }

    // 버튼 이벤트용 생성자
    FXboxControllerEvent(
        const EXboxButtons::Type InButton,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent,
        const uint32 InControllerId,
        const bool bInIsRepeat = false
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , Button(InButton)
        , AnalogType(EXboxAnalog::Invalid)
        , AnalogValue(0.0f)
        , ControllerId(InControllerId)
        , bIsRepeat(bInIsRepeat)
        , bIsAnalogEvent(false)
    {
    }

    // 아날로그 이벤트용 생성자
    FXboxControllerEvent(
        const EXboxAnalog::Type InAnalogType,
        const FModifierKeysState& InModifierKeys,
        EInputEvent InInputEvent,
        const float InAnalogValue,
        const uint32 InControllerId
    )
        : FInputEvent(InModifierKeys, InInputEvent)
        , Button(EXboxButtons::Invalid)
        , AnalogType(InAnalogType)
        , AnalogValue(InAnalogValue)
        , ControllerId(InControllerId)
        , bIsRepeat(false)
        , bIsAnalogEvent(true)
    {
    }

public:
    EXboxButtons::Type GetButton() const
    {
        return Button;
    }

    EXboxAnalog::Type GetAnalogType() const
    {
        return AnalogType;
    }

    float GetAnalogValue() const
    {
        return AnalogValue;
    }

    uint32 GetControllerId() const
    {
        return ControllerId;
    }

    bool IsRepeat() const
    {
        return bIsRepeat;
    }

    bool IsAnalogInput() const
    {
        return bIsAnalogEvent;
    }

    bool IsButtonInput() const
    {
        return !bIsAnalogEvent;
    }

    virtual bool IsControllerEvent() const override { return true; }

private:
    EXboxButtons::Type Button;
    EXboxAnalog::Type AnalogType;
    float AnalogValue;
    uint32 ControllerId;
    bool bIsRepeat;
    bool bIsAnalogEvent;
};
