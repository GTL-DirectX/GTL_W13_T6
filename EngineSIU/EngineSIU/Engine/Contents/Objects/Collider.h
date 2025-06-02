#pragma once

#include "Engine/StaticMeshActor.h"

class ACollider : public AStaticMeshActor
{
    DECLARE_CLASS(ACollider, AStaticMeshActor)

public:
    ACollider() = default;

    virtual void PostSpawnInitialize() override;


};

