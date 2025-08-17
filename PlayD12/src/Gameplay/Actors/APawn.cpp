#include "PCH.h"

#include "APawn.h"

#include "Gameplay/Components/CameraComponent.h"

#include "Gameplay/Actors/AController.h"
 
#include "Gameplay/ActorComponent.h"

namespace Gameplay {


	APawn::APawn()
	{
		this->InputComponent = CreateActorComponent<UInputComponent>();
		 
		//todo: init default;
		this->SpringArmComponent = this->CreateComponentAsSubObject<USpringArmComponent>();
		this->CameraComponent = this->CreateComponentAsSubObject<UCameraComponent>();
		//attach:
		//this->RootComponent = CreateComponentAsSubObject<USceneComponent>();
		//this->SpringArmComponent->AttachTo(RootComponent.get());
		//this->CameraComponent->AttachTo(SpringArmComponent.get());

	}

	//responses when possessed;
	void APawn::PossessedBy(AController* newController)
	{
		Controller = newController;

		newController->SetInputComponent(InputComponent.get());
	}

}