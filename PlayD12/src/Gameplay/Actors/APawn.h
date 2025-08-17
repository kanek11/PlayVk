#pragma once
#include "Gameplay/Actor.h" 

#include "Gameplay/Components/CameraComponent.h"

namespace Gameplay
{
    class AController;
	class UInputComponent;

    class APawn : public AActor {
    public:
        APawn();

        virtual void PossessedBy(AController* NewController);

        virtual void UnPossessed() {
            Controller = nullptr;
        }

        //virtual void OnTick(float DeltaTime) override;

        /*Input bindings*/
        //virtual void SetupPlayerInputComponent() {
        //   
        //}

        //bind to player; optional;
        //int32_t AutoPossessPlayer = -1; // -1,0 local ,1 


    public:
        AController* Controller{ nullptr };

        SharedPtr<USpringArmComponent> SpringArmComponent;
        SharedPtr<UCameraComponent> CameraComponent;

        //unregistered, controlled by PC;
        SharedPtr<UInputComponent> InputComponent;
    };
}