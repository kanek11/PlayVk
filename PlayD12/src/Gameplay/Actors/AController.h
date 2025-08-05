#pragma once


#include "Gameplay/Actor.h"

namespace Gameplay
{ 
	class APawn;

	//<<abstract>>
	class AController : public AActor
	{ 
	public:
		//virtual void OnTick(float DeltaTime) override; 
		virtual void Possess(APawn* Pawn);
		virtual void UnPossess();  

		//void GetPlayerViewPoint(FVector& OutLocation, FRotator& OutRotation) const;
		//AActor* GetViewTarget() const;

		APawn* Pawn;
	};
}