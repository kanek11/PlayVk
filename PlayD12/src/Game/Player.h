#pragma once

#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

#include "Physics/PhysicsScene.h"

#include "Gameplay/Actors/AController.h"

#include "Ability.h"

#include "Math/GenericUtils.h"

#include "Gameplay/Actors/StaticMeshActor.h"

using namespace Gameplay;


struct FPlayerState {

	float speed;
	float accel;

	EPlayerForm currForm;
	std::unordered_map<EPlayerForm, FAbilityRuntime> abilitiesRT;
};



class APlayer : public APawn
{
public:
	APlayer();
	virtual void BeginPlay() override;
	virtual void EndPlay() override;

	virtual void OnTick(float delta) override;

	virtual void OnRegister();

private: 
	void RequestTransitForm(EPlayerForm form);
	void TransitForm(EPlayerForm form);

	void DuringFormTransition(float nt, float startSize);


	void TickPlayingPersistent(float delta);

	void NormalStateBehavior(float delta);
	void MetalStateBehavior(float delta);
	void IceStateBehavior(float delta);

	void CloneStateBehavior(float delta);

private:
	void OnOverlapItem(AActor* other);

private:
	void UploadPlayerState();

public:
	SharedPtr<UStaticMeshComponent> staticMeshComponent;
	SharedPtr<UShapeComponent> shapeComponent; 

public:
	FPlayerState playerState{}; 


public:
	void AddPayloadImmediate(const FAbilityPayload& payload); 

	EPlayerForm currForm;
	std::unordered_map<EPlayerForm, FAbilityRuntime> abilities; 

	//todo : define elsewhere?
	std::unordered_map<EPlayerForm, FFormState> playerForms;


public:
	BoolEdgeDetector EDGrounded{}; 
	bool bGroundedThisFrame{ false };

	void RegisterPhysicsHooks();

public:
	float playerOGR = 1.0f;
	float playerShrinkR = 0.3f;
	float particleR = 0.1f;
	std::vector<Float3> particlePos;
	std::vector<SharedPtr<AStaticMeshActor>> particleActors;

	void SpawnParticles();
	void ShowParticles();
	void HideParticles();

	void MoveParticles(float nt);


	float emitDuration = 0.5f;

	float bDisableTransition{ false };

	bool bToReset{ false };
};