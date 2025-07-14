#pragma once

#include "Base.h"
#include "UI.h"
class IWorld {
public:
	virtual ~IWorld() = default;
	virtual void OnLoad() = 0;
	virtual void OnUnload() = 0;
	virtual void OnUpdate(float delta) = 0;
};


class WorldManager {

	void TransitWorld(SharedPtr<IWorld> world) {
		if (currentWorld) {
			currentWorld->OnUnload();
		}
		currentWorld = world;

		if (currentWorld) {
			currentWorld->OnLoad();
		}
	}

	void Update(float delta) {
		if (currentWorld) {
			currentWorld->OnUpdate(delta);
		}
	}

private:
	SharedPtr<IWorld>  currentWorld;
};


class GamePlayWorld {
public:
	virtual ~GamePlayWorld() = default;
	virtual void OnLoad();
	virtual void OnUnload();
	virtual void OnUpdate(float delta);
};


class MainMenuWorld {
public:
	virtual ~MainMenuWorld() = default;
	virtual void OnLoad();
	virtual void OnUnload();
	virtual void OnUpdate(float delta);

	SharedPtr<UIButton> debugButton;
};