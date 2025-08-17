
#include "PCH.h"
#include "World.h" 

#include "Application.h"

#include "Physics/PhysicsEvent.h"

namespace Gameplay {

	void UWorld::Init()
	{
		auto& gTime = GameApplication::GetInstance()->GetTimeSystem();
		physicsScene = new PhysicsScene();
		physicsScene->OnInit();

		//physics update is handled by time system;
		gTime.RegisterFixedFrame([=](float delta) {
			physicsScene->Tick(delta);
			}); 

		//new understanding:  controllers are managed by world itself; 
		auto dftPlayerController = CreateActor<AController>();
		this->AddPlayerController(dftPlayerController);


		this->persistentLevel->OnLoad();
		this->persistentLevel->RouteActorBeginPlay();
	}

	void UWorld::BeginPlay()
	{
		if (currentLevel) {
			currentLevel->RouteActorBeginPlay();
		}

		for (auto& controller : playerControllers) {
			controller->BeginPlay();
		}
	}
	void UWorld::EndPlay()
	{
		if (currentLevel) {
			currentLevel->EndPlay();
		}

		for (auto& controller : playerControllers) {
			controller->EndPlay();
		}
	}

	void UWorld::SyncPhysicsToGame()
	{
		//if (currentLevel) {
		//	currentLevel->SyncPhysicsToGame();
		//}
		this->DispatchPhysicsEvents(); 

		auto& transformBuffer = physicsScene->GetTransformBuffer();
		for (auto& [id, trans] : transformBuffer) {

			if (!this->m_primtiveMap.contains(id)) continue; 
			auto& primitive = this->m_primtiveMap.at(id);

			if (!primitive->IsSimulatingPhysics()) continue;

			primitive->SetRelativePosition(trans.position);
			primitive->SetRelativeRotation(trans.rotation);

			//std::cout << "update physics pos:" << ToString(trans.position) << " for id:" << id << '\n'; 
		}
	}

	void UWorld::DispatchPhysicsEvents()
	{
		auto& events = PhysicsEventQueue::Get().Drain();
		for (auto& event : events) {
			auto actorA = this->PrimitiveIdToActor(event.a_ID);
			auto actorB = this->PrimitiveIdToActor(event.b_ID);

			m_primtiveMap[event.a_ID]->onOverlap.BlockingBroadCast(actorB);
			m_primtiveMap[event.b_ID]->onOverlap.BlockingBroadCast(actorA);
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
		if (currentLevel) {
			//std::cout << "tick current world: " << currentWorld << '\n';
			currentLevel->OnTick(delta);
		}

		for (auto& controller : playerControllers) {
			controller->OnTick(delta);
		}


		//
		this->ConstructSceneView();

		//
		scene.SubmitAll();
	}


}