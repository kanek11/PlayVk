#pragma once

#include "Gameplay/Level.h"

using namespace Gameplay;

class GamePlayLevel : public ULevel {
public:
	virtual ~GamePlayLevel() = default;
	virtual void OnLoad() override;
	virtual void OnUnload()override;
	virtual void OnTick(float delta) override;

	void LoadUI(); 
	void LoadActors(); 

	void LoadLegacy();

private:
	//std::vector<SharedPtr<StaticMeshActorProxy>> m_staticMeshActors;
	std::unordered_map<ActorId, SharedPtr<StaticMeshActorProxy>> m_staticMeshActors;
	 
public: 
	//void GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount);
	float roadWidth = 10;
	float goalLength = 50;

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


