#pragma once
#include <algorithm>

#include "Math/Vector.h"
#include "UObject/ObjectMacros.h"

struct FDistributionVector
{
    DECLARE_STRUCT(FDistributionVector)

    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, MinValue)
    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, MaxValue)

public:
    FDistributionVector()
        : MinValue(FVector::ZeroVector)
        , MaxValue(FVector::ZeroVector)
    {}

    // 범위를 지정하는 생성자
    FDistributionVector(const FVector& InMin, const FVector& InMax)
        : MinValue(InMin)
        , MaxValue(InMax)
    {   }

    void UpdateRange(const FVector& InMin, const FVector& InMax)
    {
        MinValue = InMin;
        MaxValue = InMax;
    }

    FVector GetValue()
    {
        float RandX = FMath::FRandRange(MinValue.X, MaxValue.X);
        float RandY = FMath::FRandRange(MinValue.Y, MaxValue.Y);
        float RandZ = FMath::FRandRange(MinValue.Z, MaxValue.Z);

        FVector RandomVector = FVector(RandX, RandY, RandZ);
        return RandomVector;
    }

    void GetOutRange(FVector& OutMin, FVector& OutMax) const;

    friend FArchive& operator<<(FArchive& Ar, FDistributionVector& DV)
    {
        Ar << DV.MinValue << DV.MaxValue;

        return Ar;
    }
};
