#pragma once 
#include "PCH.h"

#include "Gameplay/Actor.h"

#include "Math/MMath.h"

namespace Gameplay
{

	enum class EInputMode
	{
		None,
		//GameOnly,  
		UIOnly, 
		//GameAndUI  //our game doesn't have that.
	};



	using InputBehavior = std::function<void(float)>;

	class UInputComponent : public UActorComponent
	{
	public:
		virtual void TickComponent(float delta) override
		{
			if(inputBehavior)
			{
				inputBehavior(delta);
			}
		}

		//braindead ver of binding 
		void BindBehavior(const InputBehavior& behavior)
		{
			inputBehavior = behavior;
		} 

		InputBehavior inputBehavior;
	}; 



	class APawn;

	//<<abstract>>
	class AController : public AActor
	{
	public:
		virtual void BeginPlay() override;

		virtual void OnTick(float delta);

		virtual void ProcessPlayerInput(float delta);


		//virtual void OnTick(float DeltaTime) override; 
		virtual void Possess(APawn* Pawn);
		virtual void UnPossess();

		void GetPlayerViewPoint(Float3& OutLocation, DirectX::XMVECTOR& OutRotation) const;

		//usually returns possessed pawn,but open to customization;
		AActor* GetViewTarget() const;


		void SetInputComponent(UInputComponent* InInputComponent)
		{
			CurrInputComp = InInputComponent;
		}

		void SetInputMode(EInputMode mode)
		{
			currInputMode = mode;
		}

	public:
		APawn* Pawn;
		UInputComponent* CurrInputComp;

		EInputMode currInputMode{ EInputMode::None };
	};
}