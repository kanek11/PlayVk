#pragma once


#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

using namespace Gameplay;

class APlayer : public APawn
{
public:
	APlayer();
	virtual void BeginPlay() override;

	virtual void OnTick(float DeltaTime) override; 

public:
	SharedPtr<UStaticMeshComponent> staticMeshComponent;
	SharedPtr<UShapeComponent> shapeComponent;
};