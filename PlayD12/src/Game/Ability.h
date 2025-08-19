#pragma once


#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

#include "Physics/PhysicsScene.h"

#include "Gameplay/Actors/AController.h"

using namespace Gameplay;


enum class EPlayerForm {
	Normal,
	MetalBall,
	IceCube,
};

struct FFormState {
	SharedPtr<UStaticMesh> mesh;
	SharedPtr<UMaterial> material;
	ShapeType shape;
	PhysicalMaterial physMaterial;
	float mass{ 1.0f };
	//
	InputBehavior inputCb;
};
 

class APlayer;
using FAbilityEffect = std::function<void(APlayer*)>; 

class FAbilityPayload {
public:
	//std::string name;
	EPlayerForm formType;
	float duration{ 1.0f };

	//FAbilityEffect OnApply;
	//FAbilityEffect OnExpire;
};


struct FAbilityRuntime {
	float remaining{ 100000.0f };
};


//class UPowerComponent : public UActorComponent {
//public:
//
//	virtual void TickComponent(float delta) override;
//
//
//};
