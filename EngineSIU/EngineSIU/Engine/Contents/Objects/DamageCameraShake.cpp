
#include "DamageCameraShake.h"

#include "Camera/PerlinNoiseCameraShakePattern.h"
#include "Camera/Shakes/WaveOscillatorCameraShakePattern.h"

UDamageCameraShake::UDamageCameraShake()
{
    UPerlinNoiseCameraShakePattern* Pattern = new UPerlinNoiseCameraShakePattern();

    // 전체 지속 시간 및 블렌드 설정
    Pattern->Duration = 0.5f;
    Pattern->BlendInTime = 0.f;
    Pattern->BlendOutTime = 0.f;

    // Rotation (회전) 축 강도 조정
    Pattern->RotationAmplitudeMultiplier = 0.5f;   // 전체 회전 강도 비율 감소
    Pattern->RotationFrequencyMultiplier = 1.f;

    Pattern->Yaw.Amplitude = 1.4f;   // 기존 3.0f → 1.5f
    Pattern->Yaw.Frequency = 5.f;

    Pattern->Pitch.Amplitude = 2.5f; // 기존 5.0f → 2.5f
    Pattern->Pitch.Frequency = 2.f;

    Pattern->Roll.Amplitude = 1.5f;  // 기존 3.0f → 1.5f
    Pattern->Roll.Frequency = 5.f;

    // Location (위치) 축 강도 조정
    Pattern->LocationAmplitudeMultiplier = 1.2f;   // 전체 위치 강도 비율 추가/조정
    Pattern->LocationFrequencyMultiplier = 1.2f;

    Pattern->X.Amplitude = 0.f;
    Pattern->X.Frequency = 1.f;

    Pattern->Y.Amplitude = 2.f;     // 기존 5.0f → 2.0f
    Pattern->Y.Frequency = 2.f;

    Pattern->Z.Amplitude = 2.f;
    Pattern->Z.Frequency = 5.f;

    SetRootShakePattern(Pattern);
}

UHitCameraShake::UHitCameraShake()
{
    UPerlinNoiseCameraShakePattern* Pattern = new UPerlinNoiseCameraShakePattern();

    // 전체 지속 시간 및 블렌드 설정
    Pattern->Duration = 0.25f;       // 기존 0.5f → 0.25f로 단축
    Pattern->BlendInTime = 0.f;
    Pattern->BlendOutTime = 0.f;

    // Rotation (회전) 축 강도 조정
    Pattern->RotationAmplitudeMultiplier = .5f; // 기존 0.5f → 1.0f로 증가
    Pattern->RotationFrequencyMultiplier = 1.f;

    Pattern->Yaw.Amplitude = 1.0f;   // 기존 1.4f → 6.0f로 증가
    Pattern->Yaw.Frequency = 1.f;   // 강도를 높이면서 빈도도 증가

    Pattern->Pitch.Amplitude = 2.0f;  // 기존 2.5f → 10.0f로 증가
    Pattern->Pitch.Frequency =1.f;    // 적절한 빈도 조정

    Pattern->Roll.Amplitude = 2.0f;   // 기존 1.5f → 6.0f로 증가
    Pattern->Roll.Frequency = 1.f;   // 높은 빈도로 빠르게 흔들리도록 설정

    // Location (위치) 축 강도 조정
    Pattern->LocationAmplitudeMultiplier = 2.0f; // 기존 1.2f → 2.0f로 증가
    Pattern->LocationFrequencyMultiplier = 1.5f; // 빈도 약간 증가

    Pattern->X.Amplitude = 0.f;
    Pattern->X.Frequency = 1.f;

    Pattern->Y.Amplitude = 2.0f;   // 기존 2.0f → 4.0f로 증가
    Pattern->Y.Frequency = 2.f;    // 빈도 증가

    Pattern->Z.Amplitude = 2.0f;   // 기존 0.0f → 2.0f로 설정
    Pattern->Z.Frequency = 6.f;    // 빈도를 높여 짧은 시간에 위치 흔들림이 일어나도록 설정

    SetRootShakePattern(Pattern);
}
