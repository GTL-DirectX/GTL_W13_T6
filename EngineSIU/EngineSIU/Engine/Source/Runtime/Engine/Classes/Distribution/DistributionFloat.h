#pragma once
#include "UObject/ObjectMacros.h"

struct FDistributionFloat
{
    DECLARE_STRUCT(FDistributionFloat)

    FDistributionFloat() : MinValue(1), MaxValue(1) {}
    FDistributionFloat(float InMinValue, float InMaxValue) : MinValue(InMinValue), MaxValue(InMaxValue) {}

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MinValue)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MaxValue)

    //float MinValue;
    //float MaxValue;

    float GetValue();
    void GetOutRange(float& OutMin, float& OutMax) const;

    friend FArchive& operator<<(FArchive& Ar, FDistributionFloat& DF)
    {
        return Ar << DF.MinValue << DF.MaxValue;
    }
};
