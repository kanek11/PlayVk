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
	void LoadUI();
	void LoadActors();
	 
public:
	//void GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount);
	float roadWidth = 20;
	float goalLength = 20;

	float timeCount{}; 
public:
	void SyncGameToPhysics() override;
	//void SyncPhysicsToGame() override;
};


class MainMenuLevel : public ULevel {
public:
	virtual ~MainMenuLevel() = default;
	virtual void OnLoad()override;
	virtual void OnUnload()override;
	virtual void OnTick(float delta) override;
};

