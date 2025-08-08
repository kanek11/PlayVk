#include "PCH.h"
#include "Player.h"

#include "Application.h"

#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"


void TransitPlayerState(AActor* actor, const FFormState& state)
{  
	actor->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
 
	if (auto meshComp = actor->GetComponent<UStaticMeshComponent>()) {
		meshComp->SetMesh(state.mesh);
		meshComp->SetMaterial(state.material);
	}

	if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
		shapeComp->rigidBody->ClearRotation();
		shapeComp->rigidBody->SetPhysicalMaterial(state.physMaterial);
		shapeComp->SetShape(state.shape);

	}
}

APlayer::APlayer() : APawn()
{ 
	this->tag = "player";

	this->staticMeshComponent = this->CreateComponentAsSubObject<UStaticMeshComponent>(); 
	this->shapeComponent = this->CreateComponentAsSubObject<UShapeComponent>();

	//Mesh::SetSphere(this, 2.0f); 

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
	auto overlapCb = [=](AActor* other) {
		std::cout << "player: overlap with tag: " << other->tag << '\n';
		};

	this->shapeComponent->onOverlap.Add(overlapCb); 


	FFormState sphereForm = {
	.mesh = CreateShared<SphereMesh>(),
	.material = Materials::GetRustyIron(), //CreateShared<UMaterial>(),
	.shape = Sphere{ 1.0f }, 
	.physMaterial = PhysicalMaterial{ 0.0f, 0.8f }
	};
	playerForms[EPlayerForm::Sphere] = sphereForm;


	FFormState cubeForm = {
		.mesh = CreateShared<CubeMesh>(),
		.material = Materials::GetIceSurface(), //CreateShared<UMaterial>(),
		.shape = Box({ 1.0f ,1.0f, 1.0f }),
		.physMaterial = PhysicalMaterial{ 0.0f, 0.0f }
	};
	playerForms[EPlayerForm::Cube] = cubeForm; 

	TransitPlayerState(this, playerForms.at(EPlayerForm::Sphere));
}

void APlayer::OnTick(float delta)
{
	APawn::OnTick(delta);

	playerState.speed = Length(this->shapeComponent->rigidBody->linearVelocity);

	auto gameState = GetWorld()->GetGameState<AGameState>();
	gameState->SetPlayerState(this->playerState);


	 




	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);

	
	if (inputSystem->IsKeyJustPressed(KeyCode::Num1))
	{
		TransitPlayerState(this, playerForms.at(EPlayerForm::Sphere));
		//Mesh::SetSphere(this,1.0f);
	} 
	else if (inputSystem->IsKeyJustPressed(KeyCode::Num2)) {
		//Mesh::SetBox(this, Float3(1.0f,1.0f,1.0));
		TransitPlayerState(this, playerForms.at(EPlayerForm::Cube));
	}
	 

}