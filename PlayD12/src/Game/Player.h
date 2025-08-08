#pragma once
 
#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

#include "Physics/PhysicsScene.h"


struct FPlayerState {

	float speed;
};

enum class EPlayerForm {
	Sphere,
	Cube,
};

struct FFormState {
	SharedPtr<UStaticMesh> mesh;
	SharedPtr<UMaterial> material;
	ShapeType shape;
	PhysicalMaterial physMaterial;
};


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

public: 
	std::unordered_map<EPlayerForm, FFormState> playerForms;

public:
	FPlayerState playerState;
};