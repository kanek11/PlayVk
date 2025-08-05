#pragma once
#include "Gameplay/Actor.h" 

#include "Gameplay/Components/CameraComponent.h"

namespace Gameplay
{ 
     
class AController; 

class APawn : public AActor {
public:
    APawn() : Controller(nullptr) {}
     
    AController* Controller;
     
    virtual void PossessedBy(AController* NewController) {
        Controller = NewController;
		//response to possession
    }
     
    virtual void UnPossessed() {
        Controller = nullptr;
    }
     
    virtual void OnTick(float DeltaTime) override {
    
    }
     
    /*Input bindings*/
    //virtual void SetupPlayerInputComponent() {
    //   
    //}

    //bind to player; optional;
    //int32_t AutoPossessPlayer = -1; // -1,0 local ,1 
     

public: 
	SharedPtr<UCameraComponent> CameraComponent; 
};
}