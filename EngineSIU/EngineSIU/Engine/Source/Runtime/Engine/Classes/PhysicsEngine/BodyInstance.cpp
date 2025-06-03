#include "BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/PhysicsManager.h"


FBodyInstance::FBodyInstance(UPrimitiveComponent* InOwner) : OwnerComponent(InOwner)
{
    
}

void FBodyInstance::SetGameObject(GameObject* InGameObject)
{
    BIGameObject = InGameObject;
}

void FBodyInstance::AddForce(const FVector& Force, bool bAccelChange) const
{
    if (!BIGameObject || !BIGameObject->DynamicRigidBody)
    {
        return;
    }
    if (bAccelChange)
    {
        // Force를 가속도로 변환
        PxVec3 Acceleration = PxVec3(Force.X, Force.Y, Force.Z) / BIGameObject->DynamicRigidBody->getMass();
        BIGameObject->DynamicRigidBody->addForce(Acceleration, PxForceMode::eACCELERATION);
        return;
    }
    BIGameObject->DynamicRigidBody->addForce(PxVec3(Force.X, Force.Y, Force.Z));
}

