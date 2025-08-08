#include "PCH.h"
#include "APawn.h"

#include "Gameplay/Components/CameraComponent.h"

namespace Gameplay {


	APawn::APawn()
	{
		//todo: init default;
		this->SpringArmComponent = CreateComponentAsSubObject<USpringArmComponent>();
		this->CameraComponent = CreateComponentAsSubObject<UCameraComponent>();
		//attach:
		//this->RootComponent = CreateComponentAsSubObject<USceneComponent>();
		//this->SpringArmComponent->AttachTo(RootComponent.get());
		//this->CameraComponent->AttachTo(SpringArmComponent.get());

	}

}