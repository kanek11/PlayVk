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
		auto owningWorld = this->GetWorld();
		owningWorld->AddPrimitiveComponent(this); 
	}


	void UPrimitiveComponent::EndPlay()
	{
		auto owningWorld = this->GetWorld();
		owningWorld->RemovePrimitiveComponent(this);
	}
}
