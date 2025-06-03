
#include "DamageCameraShake.h"

#include "Camera/PerlinNoiseCameraShakePattern.h"
#include "Camera/Shakes/WaveOscillatorCameraShakePattern.h"

UDamageCameraShake::UDamageCameraShake()
{
    UPerlinNoiseCameraShakePattern* Pattern = new UPerlinNoiseCameraShakePattern();

    // 전체 지속 시간 및 블렌드 설정
    Pattern->Duration = 0.1f;
    Pattern->BlendInTime = 0.f;
    Pattern->BlendOutTime = 0.f;

    // Rotation (회전) 축 강도 조정
    Pattern->RotationAmplitudeMultiplier = 0.2f;   // 전체 회전 강도 비율 감소
    Pattern->RotationFrequencyMultiplier = 1.f;

    Pattern->Yaw.Amplitude = .6f;   // 기존 3.0f → 1.5f
    Pattern->Yaw.Frequency = 5.f;

    Pattern->Pitch.Amplitude = 2.5f; // 기존 5.0f → 2.5f
    Pattern->Pitch.Frequency = 2.f;

    Pattern->Roll.Amplitude = 1.5f;  // 기존 3.0f → 1.5f
    Pattern->Roll.Frequency = 5.f;

    // Location (위치) 축 강도 조정
    Pattern->LocationAmplitudeMultiplier = 0.2f;   // 전체 위치 강도 비율 추가/조정
    Pattern->LocationFrequencyMultiplier = 1.f;

    Pattern->X.Amplitude = 0.f;
    Pattern->X.Frequency = 1.f;

    Pattern->Y.Amplitude = 2.f;     // 기존 5.0f → 2.0f
    Pattern->Y.Frequency = 2.f;

    Pattern->Z.Amplitude = 0.f;
    Pattern->Z.Frequency = 5.f;

    SetRootShakePattern(Pattern);
}
