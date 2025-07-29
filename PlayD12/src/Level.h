#pragma once

#include "Base.h"
#include "UI.h"

#include "StaticMeshActor.h"

namespace Global {
	static float lastUsedTime = std::numeric_limits<float>::max();
} 

class ILevel {
public:
	virtual ~ILevel() = default;
	virtual void OnLoad() = 0;
	virtual void OnUnload() = 0;
	virtual void OnUpdate(float delta) = 0;
};


class WorldManager {
public:

	void RegisterWorld(std::string name, SharedPtr<ILevel> world) {
		levels[name] = world;
	}

	void TransitWorld(std::string name) {
		if (currentWorld) {
			currentWorld->OnUnload();
		}

		if (!levels.contains(name)) {
			std::cerr << "world name not found" << std::endl;
			return;
		}

		std::cout << "set current world: " << name << '\n';
		currentWorld = levels[name];

		if (currentWorld) {
			currentWorld->OnLoad();
		}
	}

	void Update(float delta) {
		if (currentWorld) {
			//std::cout << "tick current world: " << currentWorld << '\n';
			currentWorld->OnUpdate(delta);
		}
	}
private:
	SharedPtr<ILevel> currentWorld;
	std::unordered_map<std::string, SharedPtr<ILevel>>  levels;
};


class GamePlayWorld : public ILevel {
public:
	virtual ~GamePlayWorld() = default;
	virtual void OnLoad() override;
	virtual void OnUnload()override;
	virtual void OnUpdate(float delta)override;

private:
	std::vector<SharedPtr<StaticMeshActorProxy>> m_staticMeshActors;
	std::vector < SharedPtr<UIButton>> m_HUDs;

public:
	FCameraProxy* dummyCamera = new FCameraProxy();
	//FollowCameraProxy* dummyCamera = new FollowCameraProxy();

public:

	void GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount);
	float roadWidth = 10;
	float goalLength = 500;

	float timeCount{};
};


class MainMenuWorld : public ILevel {
public:
	virtual ~MainMenuWorld() = default;
	virtual void OnLoad()override;
	virtual void OnUnload()override;
	virtual void OnUpdate(float delta)override;

	std::vector < SharedPtr<UIButton>> m_Buttons;
};