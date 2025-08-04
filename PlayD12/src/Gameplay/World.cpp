
#include "PCH.h"
#include "World.h" 

#include "Application.h"

namespace Gameplay {


	void UWorld::Init()
	{ 
		auto& gTime = GameApplication::GetInstance()->GetTimeSystem();
		physicsScene = new PhysicsScene();
		physicsScene->OnInit();

		gTime.RegisterFixedFrame([=](float delta) {
			physicsScene->Tick(delta);
			});
	 
	}
	void UWorld::SyncPhysicsToGame()
	{
		//if (currentLevel) {
		//	currentLevel->SyncPhysicsToGame();
		//}
		  
		auto& transformBuffer = physicsScene->GetTransformBuffer();
		for (auto& [id, trans] : transformBuffer) {

			if (!this->m_primtiveMap.contains(id)) continue;
			auto& owner = this->m_primtiveMap.at(id);

			owner->SetRelativePosition(trans.position);
			owner->SetRelativeRotation(trans.rotation);

			//std::cout << "update physics pos:" << ToString(trans.position) << " for id:" << id << '\n';

		}
	}
}