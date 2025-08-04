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
#include "StaticMeshActor.h"
 

namespace Global {
	static float lastUsedTime = std::numeric_limits<float>::max();
} 

namespace Gameplay{ 

using sceneIterFn = std::function<void(USceneComponent*)>;
 

class UWorld;

class ULevel {
public:
	virtual ~ULevel() = default;
	virtual void OnLoad() ;
	virtual void OnUnload() = 0;
	virtual void OnUpdate(float delta);

	virtual void SyncGameToPhysics() {}; 

public: 
	//new:
	std::vector<SharedPtr<AActor>> m_actors;

	void AddActor(SharedPtr<AActor> actor);

	void UpdateTransforms();

	void ForEachComponent(const sceneIterFn& fn);
	void TraverseTree(USceneComponent* comp, const sceneIterFn& fn);
	 

	//dummy design: 
	FCameraProxy defaultCamera;

public:
	UWorld* owningWorld;
}; 
}