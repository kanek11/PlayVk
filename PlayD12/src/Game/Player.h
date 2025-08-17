#pragma once
 
#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

#include "Physics/PhysicsScene.h"

#include "Gameplay/Actors/AController.h"

using namespace Gameplay;

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

	//
	InputBehavior inputCb;
};


using namespace Gameplay;

class APlayer : public APawn
{
public:
	APlayer();
	virtual void BeginPlay() override;

	virtual void OnTick(float delta) override;

	void UploadPlayerState();

	void SphereStateBehavior(float delta);
	void CubeStateBehavior(float delta);

public:
	SharedPtr<UStaticMeshComponent> staticMeshComponent;
	SharedPtr<UShapeComponent> shapeComponent; 

public: 
	std::unordered_map<EPlayerForm, FFormState> playerForms;

public:
	FPlayerState playerState{};
};