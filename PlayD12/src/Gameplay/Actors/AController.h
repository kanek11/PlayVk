#pragma once 
#include "PCH.h"

#include "Gameplay/Actor.h"

#include "Math/MMath.h"

namespace Gameplay
{
	class APawn;

	//<<abstract>>
	class AController : public AActor
	{
	public:
		virtual void BeginPlay() override;

		//virtual void OnTick(float DeltaTime) override; 
		virtual void Possess(APawn* Pawn);
		virtual void UnPossess();

		void GetPlayerViewPoint(Float3& OutLocation, DirectX::XMVECTOR& OutRotation) const;

		//usually returns possessed pawn,but open to customization;
		AActor* GetViewTarget() const;

		APawn* Pawn;
	};
}