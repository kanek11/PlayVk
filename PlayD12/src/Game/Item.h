#pragma once

#include "Gameplay/Actors/StaticMeshActor.h"

class ABoxItem : public AStaticMeshActor {
public:
	ABoxItem();
	 
	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override;

};


class ATriggerVolume : public AStaticMeshActor {
public:
	ATriggerVolume();

	virtual void BeginPlay() override;
	virtual void OnTick(float delta) override;

};