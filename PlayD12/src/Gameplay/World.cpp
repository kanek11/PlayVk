
#include "PCH.h"
#include "World.h" 

#include "Application.h"

#include "Physics/PhysicsEvent.h"

namespace Gameplay {

	void UWorld::Init()
	{
		auto gTime = GameApplication::GetInstance()->GetTimeSystem();
		physicsScene->OnInit();

		//physics update is handled by time system;
		gTime->RegisterFixedFrame([=](float delta) {
			physicsScene->Tick(delta);
			this->SyncPhysicsToGame();
			});

		//new understanding:  controllers are managed by world itself; 
		auto dftPlayerController = CreateActor<AController>();
		this->AddPlayerController(dftPlayerController);
	}

	void UWorld::BeginPlay()
	{
		this->persistentLevel->OnLoad();
		this->persistentLevel->RouteActorBeginPlay();

		if (currentLevel) {
			currentLevel->RouteActorBeginPlay();
		}

		for (auto& controller : playerControllers) {
			controller->BeginPlay();
		}

		this->bGameBegined = true;
	}
	void UWorld::EndPlay()
	{
		this->persistentLevel->OnUnload();
		this->persistentLevel->RouteActorEndPlay();

		if (currentLevel) {
			currentLevel->RouteActorEndPlay();
		}

		for (auto& controller : playerControllers) {
			controller->EndPlay();
		}
	}

	void UWorld::SyncPhysicsToGame()
	{

		for (auto& [id, prim] : this->m_primtiveMap) {

			//std::cout << "prim id:" << id << '\n';
			prim->onPrePhysicsEvents.BlockingBroadCast();
		}

		this->DispatchPhysicsEvents();

		// 
		auto& transformBuffer = physicsScene->GetTransformBuffer();
		for (auto& [id, trans] : transformBuffer) {

			if (!this->m_primtiveMap.contains(id)) continue;
			auto& primitive = this->m_primtiveMap.at(id);

			if (!primitive->IsSimulatingPhysics()) continue;

			//std::cout << "prim id:" << id << '\n';

			primitive->SetRelativePosition(trans.position);
			primitive->SetRelativeRotation(trans.rotation);

			//std::cout << "update physics pos:" << ToString(trans.position) << " for id:" << id << '\n'; 
		}
	}

	void UWorld::SyncGameToPhysics() {

		for (auto& [id, primitive] : this->m_primtiveMap) {

			physicsScene->SetPosition(id, primitive->GetWorldPosition());
			physicsScene->SetRotation(id, primitive->GetWorldRotation());
			//std::cout << "primitive component set id:" << id << " position: " << ToString(primitive->GetWorldPosition()) << '\n';
		}
	}

	void UWorld::DispatchPhysicsEvents()
	{
		auto events = PhysicsEventQueue::Get().Drain();
		for (auto& event : events) {
			auto actorA = this->PrimitiveIdToActor(event.a_ID);
			auto actorB = this->PrimitiveIdToActor(event.b_ID);

			if (m_primtiveMap.contains(event.a_ID)) {
				//std::cout << "prim id:" << event.a_ID << '\n';

				m_primtiveMap[event.a_ID]->onOverlap.BlockingBroadCast(actorB, event.contact);
			}

			if (m_primtiveMap.contains(event.b_ID)) {
				//std::cout << "prim id:" << event.b_ID << '\n';

				m_primtiveMap[event.b_ID]->onOverlap.BlockingBroadCast(actorA, event.contact);
			}
		}
	}

	//somewhat unavoidable verbosity?
	void UWorld::ConstructSceneView()
	{
		//
		auto controller = this->GetFirstPlayerController();
		if (!controller) return;

		//std::cout << "detect player controller\n";

		//
		AActor* viewer = controller->GetViewTarget();
		if (!viewer) return;

		//std::cout << "detect player\n";

		//
		auto cameraComp = viewer->GetComponent<UCameraComponent>();
		if (!cameraComp) return;

		//std::cout << "detect camera\n";

		//leave it here for now
		//auto& cameraTrans = cameraComp->GetWorldTransform(); 
		auto eyePos = cameraComp->GetWorldPosition();
		//std::cout << "camera position: " << ToString(eyePos) << '\n';

		auto view_ = cameraComp->GetViewMatrix();
		auto invView_ = cameraComp->GetInvViewMatrix();

		auto proj_ = cameraComp->GetProjectionMatrix();
		auto invProj_ = cameraComp->GetInvProjectionMatrix();

		auto pvMatrix = MMath::MatrixMultiply(proj_, view_);

		FSceneView sceneView = {
			.pvMatrix = pvMatrix,
			.invProjMatrix = invProj_,
			.invViewMatrix = invView_,
			.position = eyePos,
		};

		this->scene.AddSceneView(sceneView);
	}


	void UWorld::OnTick(float delta)
	{

		if (m_gameState) {
			m_gameState->OnTick(delta);
		}


		if (currentLevel) {
			//std::cout << "tick current world: " << currentWorld << '\n';
			currentLevel->OnTick(delta);
		}

		//tween:
		Anim::TweenRunner::Get().Update(delta);

		for (auto& controller : playerControllers) {
			controller->OnTick(delta);
		}

		//
		this->ConstructSceneView();

		//
		scene.SubmitAll();
	}


}