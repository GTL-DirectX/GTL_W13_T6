#include "DistributionVector.h"
void FDistributionVector::GetOutRange(FVector& OutMin, FVector& OutMax) const
{
    OutMin.X = FMath::Min(MinValue.X, MaxValue.X);
    OutMax.X = FMath::Max(MinValue.X, MaxValue.X);

    OutMin.Y = FMath::Min(MinValue.Y, MaxValue.Y);
    OutMax.Y = FMath::Max(MinValue.Y, MaxValue.Y);

    OutMin.Z = FMath::Min(MinValue.Z, MaxValue.Z);
    OutMax.Z = FMath::Max(MinValue.Z, MaxValue.Z);
}
