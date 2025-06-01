#include "DistributionFloat.h"

float FDistributionFloat::GetValue()
{
    float RandomValue = FMath::FRandRange(MinValue, MaxValue);
    return RandomValue;
}
void FDistributionFloat::GetOutRange(float& OutMin, float& OutMax) const
{
    OutMin = FMath::Min(MinValue, MaxValue);
    OutMax = FMath::Max(MinValue, MaxValue);
}
