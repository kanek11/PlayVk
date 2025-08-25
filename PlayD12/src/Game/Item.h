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


class ACloneItem : public AItem {
public:
	ACloneItem();

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



//class URotateComponent : UActorComponent {
//public:
//	virtual void TickComponent(float delta);
//
//	float rotationSpeed{ 10.0f };
//};


class ARotateBox : public AStaticMeshActor {
public:
	ARotateBox();
	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override;
	void SetRotationSpeed(float speed) { rotationSpeed = speed; }

private:
	float rotationSpeed{ 10.0f };
};



class AOscillateBox : public AStaticMeshActor {
public:
	AOscillateBox();
	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override; 

	float m_elapsedTime{ 0 };

	//
	float m_amplitude = 1.0f;   // 
	float m_frequency = 2.0f;    // rad/s 
	float m_phase = 0.0f;        // 

	Float3 m_startPos{};
};