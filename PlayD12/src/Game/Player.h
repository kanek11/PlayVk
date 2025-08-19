#pragma once

#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"

#include "Physics/PhysicsScene.h"

#include "Gameplay/Actors/AController.h"

#include "Ability.h"

using namespace Gameplay;


struct FPlayerState {

	float speed;
	float accel;

	std::unordered_map<EPlayerForm, FAbilityRuntime> abilitiesRT;
};



class APlayer : public APawn
{
public:
	APlayer();
	virtual void BeginPlay() override;
	virtual void EndPlay() override;

	virtual void OnTick(float delta) override;



private: 
	void RequestTransitForm(EPlayerForm form);


	void TickPlayingPersistent(float delta);

	void NormalStateBehavior(float delta);
	void MetalStateBehavior(float delta);
	void IceStateBehavior(float delta);

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
};