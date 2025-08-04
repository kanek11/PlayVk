#pragma once

#include "Gameplay/Level.h"

using namespace Gameplay;

class GamePlayWorld : public ULevel {
public:
	virtual ~GamePlayWorld() = default;
	virtual void OnLoad() override;
	virtual void OnUnload()override;
	virtual void OnUpdate(float delta) override;

	void Load1();
	void Load2();

private:
	//std::vector<SharedPtr<StaticMeshActorProxy>> m_staticMeshActors;
	std::unordered_map<ActorHandle, SharedPtr<StaticMeshActorProxy>> m_staticMeshActors;
	std::vector <SharedPtr<UIButton>> m_HUDs;
	 
public: 
	//void GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount);
	float roadWidth = 10;
	float goalLength = 50;

	float timeCount{};


public:
	void SyncGameToPhysics() override;
	//void SyncPhysicsToGame() override;
};


class MainMenuWorld : public ULevel {
public:
	virtual ~MainMenuWorld() = default;
	virtual void OnLoad()override;
	virtual void OnUnload()override;
	virtual void OnUpdate(float delta)override;

	std::vector<SharedPtr<UIButton>> m_Buttons;
};


