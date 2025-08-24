#include "PCH.h"
#include "PrimitiveComponent.h"

#include "Gameplay/World.h"
namespace Gameplay {

	std::atomic<uint32_t> UPrimitiveComponent::GComponentIdGenerator{ 0 };

	UPrimitiveComponent::UPrimitiveComponent()
	{
		id = ++GComponentIdGenerator;
	}

	void UPrimitiveComponent::OnRegister()
	{
		USceneComponent::OnRegister();

		auto owningWorld = this->GetWorld();
		owningWorld->AddPrimitiveComponent(this); 
	}


	void UPrimitiveComponent::EndPlay()
	{
		USceneComponent::EndPlay();

		auto owningWorld = this->GetWorld();
		owningWorld->RemovePrimitiveComponent(this);

		this->m_children.clear(); 
		this->m_parent = nullptr;  

		this->onOverlap.Clear();  
		this->onPrePhysicsEvents.Clear(); 


	}
}
