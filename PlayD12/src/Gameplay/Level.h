#pragma once

#include "PCH.h"
#include "Base.h"

#include "Gameplay/Actor.h"
#include "Gameplay/SceneComponent.h"
#include "Gameplay/Components/PrimitiveComponent.h"
#include "Gameplay/Components/MeshComponent.h"

#include "Gameplay/Scene.h"

#include "UI.h" 

#include "Physics/PhysicsScene.h" 

#include "Gameplay/Actors/APawn.h"
#include "Gameplay/Actors/AController.h"

namespace Global {
	static float lastUsedTime = std::numeric_limits<float>::max();
}

namespace Gameplay {

	using sceneIterFn = std::function<void(USceneComponent*)>;

	class UWorld;

	class ULevel {
	public:
		virtual ~ULevel() = default;
		virtual void OnLoad();
		virtual void OnUnload();
		virtual void OnTick(float delta);

		void RouteActorBeginPlay();
		void RouteActorEndPlay();
		 

	public:
		//new: public because level is basically container of actors
		std::vector<SharedPtr<AActor>> actors;

		void AddActor(SharedPtr<AActor> actor);

		void UpdateTransforms();

		void ForEachComponent(const sceneIterFn& fn);
		void TraverseTree(USceneComponent* comp, const sceneIterFn& fn);

	public:
		//UIs are managed separately with actors
		//std::vector<SharedPtr<UIButton>> m_buttons;

		//dummy design: 
		//FCameraProxy defaultCamera;
		//APawn* defaultPlayer; 
	public:
		UWorld* owningWorld; 
	};
}