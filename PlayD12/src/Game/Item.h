#pragma once

#include "Gameplay/Actors/StaticMeshActor.h"

#include "Ability.h"

/*
* quick convention;
* items are tagged as item;  

*/

class AItem : public AStaticMeshActor {
public:
	AItem();

	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override;

	virtual FAbilityPayload GetPlayload() {
		return payload;
	}
	FAbilityPayload payload{};
	bool bConsumed{ false };
};



class AIceItem : public AItem {
public:
	AIceItem();

	//virtual void BeginPlay() override;
	//virtual void OnTick(float delta) override;

	virtual FAbilityPayload GetPlayload() override {
		return payload;
	} 
};


class AMetalItem : public AItem {
public:
	AMetalItem();

	//virtual void BeginPlay() override;
	//virtual void OnTick(float delta) override;

	virtual FAbilityPayload GetPlayload() override {
		return payload;
	}
}; 


class ATriggerVolume : public AStaticMeshActor {
public:
	ATriggerVolume();

	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override;

};