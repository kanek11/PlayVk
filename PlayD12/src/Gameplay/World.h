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

#include "Level.h"

#include "GameStateBase.h"


namespace Gameplay {


	class UWorld {
	public:
		void Init();

		void BeginPlay();
		void EndPlay();

		void OnTick(float delta);

		template<DerivedFrom<ULevel> T>
		SharedPtr<T> CreateAndRegisterLevel(const std::string& name) {
			auto level = CreateShared<T>();
			levels[name] = level;
			level->owningWorld = this;
			return level;
		}

		void SetPersistentLevel(SharedPtr<ULevel> level) {
			persistentLevel = level;
			if (persistentLevel) {
				persistentLevel->owningWorld = this;
			}

		}

		//todo: level loading could be async or more complex;
		void LoadOrResetLevel(const std::string& name) {

			assert(levels.contains(name));

			if (currentLevelName == name) {
				std::cout << "reloading level:" << name << '\n';
				//return;
			} 

			if (currentLevel) { 
				currentLevel->RouteActorEndPlay();
				currentLevel->OnUnload();
			}

			if (!levels.contains(name)) {
				std::cerr << "world name not found" << std::endl;
				return;
			}

			std::cout << "set current world: " << name << '\n';
			currentLevel = levels[name];
			currentLevelName = name;

			if (currentLevel) {
				currentLevel->OnLoad();
				currentLevel->RouteActorBeginPlay();
			}
		}

	public:
		void SyncGameToPhysics();
		void SyncPhysicsToGame();

		void DispatchPhysicsEvents();

	private:
		std::string currentLevelName;
		SharedPtr<ULevel> currentLevel;
		std::unordered_map<std::string, SharedPtr<ULevel>> levels;

		SharedPtr<ULevel> persistentLevel; 

	public:
		PhysicsScene* physicsScene = new PhysicsScene();
		std::unordered_map<FPrimitiveComponentId, UPrimitiveComponent*> m_primtiveMap;

		void AddPrimitiveComponent(UPrimitiveComponent* comp) {
			m_primtiveMap[comp->id] = comp;
		}

		void RemovePrimitiveComponent(UPrimitiveComponent* comp) {
			m_primtiveMap.erase(comp->id);
		} 

		//todo: so verbose..
		AActor* PrimitiveIdToActor(FPrimitiveComponentId id) {
			if (m_primtiveMap.contains(id) && !m_primtiveMap.at(id)->GetOwner().expired()) {
				return m_primtiveMap.at(id)->GetOwner().lock().get();
			}
			return nullptr;
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
			auto pc = playerControllers.empty() ? nullptr : playerControllers[0].get();
			assert(pc != nullptr);
			return pc;
		}


	private:
		void ConstructSceneView();

	public:
		Gameplay::FScene scene;


	private:
		SharedPtr<AGameStateBase> m_gameState;

	public:
		template<DerivedFrom<AGameStateBase> T>
		void CreateGameState(ULevel* level) {
			//static_assert(std::is_base_of<AGameStateBase, T>::value, "Must be a GameStateBase");
			m_gameState = CreateShared<T>();
			level->AddActor(m_gameState);
		}

		template<DerivedFrom<AGameStateBase> T>
		T* GetGameState() const {
			assert(m_gameState != nullptr);
			return dynamic_cast<T*>(m_gameState.get());
		}

	public:
		//only register behavior for now
		template<DerivedFrom<AActor> T>
		T* SpawnActor(ULevel* level) {
			auto actor = CreateActor<T>();
			level->AddActor(actor);
			return actor.get();
		}

	};



}