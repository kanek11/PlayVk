#pragma once

#include "Gameplay/Level.h"

using namespace Gameplay;

class GlobalLevel : public ULevel {
public:
	virtual ~GlobalLevel() = default;
	virtual void OnLoad() override;
	virtual void OnUnload() override;
	virtual void OnTick(float delta) override;
};

class GamePlayLevel : public ULevel {
public:
	virtual ~GamePlayLevel() = default;
	virtual void OnLoad() override;
	virtual void OnUnload() override;
	virtual void OnTick(float delta) override;

	void LoadPlayer();
	void LoadActors();


	void LoadMap0();

public:
	//void GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount);
	float roadWidth = 20;
	float refLength = 100;

	float timeCount{};

	Float3 playerSpawnPos = { 0.0f, 2.0f, 1.0f };
public:
	//void SyncGameToPhysics() override;
	//void SyncPhysicsToGame() override;
};

