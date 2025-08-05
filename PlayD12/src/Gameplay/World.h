#pragma once
#include "PCH.h"
#include "Base.h"

#include "Gameplay/Actor.h"
#include "Gameplay/SceneComponent.h"
#include "Gameplay/Components/MeshComponent.h"

#include "Gameplay/Actors/AController.h"

#include "Gameplay/Scene.h"

#include "UI.h" 

#include "Physics/PhysicsScene.h"
#include "StaticMeshActor.h"

#include "Level.h"


namespace Gameplay {


	class UWorld {
	public:
		void Init();

		void BeginPlay();
		void EndPlay(); 

		void OnTick(float delta); 

		void RegisterLevel(std::string name, SharedPtr<ULevel> level) {
			levels[name] = level;
			level->owningWorld = this;
		}

		//todo: level loading could be async or more complex;
		void TransitLevel(std::string name) {
			if (currentLevel) {
				currentLevel->EndPlay();
				currentLevel->OnUnload();
			}

			if (!levels.contains(name)) {
				std::cerr << "world name not found" << std::endl;
				return;
			}

			std::cout << "set current world: " << name << '\n';
			currentLevel = levels[name];

			if (currentLevel) {
				currentLevel->OnLoad();
				currentLevel->BeginPlay();
			}
		}

		void SyncGameToPhysics() {
			if (currentLevel) {
				currentLevel->SyncGameToPhysics();
			}
		}

		void SyncPhysicsToGame();

	private:
		SharedPtr<ULevel> currentLevel;
		std::unordered_map<std::string, SharedPtr<ULevel>> levels; 

	public:
		PhysicsScene* physicsScene{ nullptr };
		std::unordered_map<FPrimitiveComponentId, UPrimitiveComponent*> m_primtiveMap;

		void AddPrimitiveComponent(UPrimitiveComponent* comp) {
			m_primtiveMap[comp->id] = comp;
		}

	public:
		//for player controller:
		std::vector<SharedPtr<AController>> playerControllers;

		void AddPlayerController(SharedPtr<AController> controller) {
			if (controller) {
				playerControllers.push_back(controller);
			}
		}

		AController* GetFirstPlayerController() const {
			return playerControllers.empty() ? nullptr : playerControllers[0].get();
		}


		void ConstructSceneView();

	public:
		Gameplay::FScene scene;
	};



}