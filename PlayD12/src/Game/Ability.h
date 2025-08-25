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
	Clone,
};

struct FFormState {
	SharedPtr<UStaticMesh> mesh;
	SharedPtr<UMaterial> material;
	ShapeType shape;
	PhysicalMaterial physMaterial;
	float mass{ 1.0f };
	float linearDamping = 1.0f;
	float angularDamping = 0.999f;
	float compliance = 0.000001f;
	//
	InputBehavior inputCb; 
	std::function<void()> onEnterTranMat;
	std::function<void()> onEnter; 

	std::function<void()> onExit;

	std::function<void(float)> duringTran;
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
	float remaining{ 10.0f };
};


//class UPowerComponent : public UActorComponent {
//public:
//
//	virtual void TickComponent(float delta) override;
//
//
//};
