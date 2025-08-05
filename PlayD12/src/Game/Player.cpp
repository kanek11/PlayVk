#include "PCH.h"
#include "Player.h"

#include "Application.h"

APlayer::APlayer() : APawn()
{ 
    auto mesh = CreateShared<SphereMesh>(); 
	this->staticMeshComponent = this->CreateComponentAsSubObject<UStaticMeshComponent>();
	this->staticMeshComponent->SetMesh(mesh); 
	 
    this->shapeComponent = this->CreateComponentAsSubObject<USphereComponent>();
	auto& rb = shapeComponent->rigidBody;
	rb->simulatePhysics = true;
	rb->simulateRotation = true;


	this->RootComponent = this->shapeComponent;
	//this->shapeComponent->AttachTo(RootComponent.get());
	this->staticMeshComponent->AttachTo(RootComponent.get()); 

    this->SpringArmComponent->AttachTo(RootComponent.get());
    this->CameraComponent->AttachTo(SpringArmComponent.get());
 
// 
    //update the world transform manually:
    RootComponent->UpdateWorldTransform();
}

void APlayer::BeginPlay()
{
}

void APlayer::OnTick(float delta)
{
	APawn::OnTick(delta); 

	auto inputSystem = GameApplication::GetInstance()->GetInputSystem(); 

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);

}
