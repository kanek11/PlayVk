#include "PCH.h"
#include "Player.h"

#include "Application.h"

#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"


void TransitPlayerState(APlayer* actor, const FFormState& targetState)
{
	actor->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());

	if (auto meshComp = actor->GetComponent<UStaticMeshComponent>()) {
		meshComp->SetMesh(targetState.mesh);
		meshComp->SetMaterial(targetState.material);
	}

	if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
		actor->RootComponent->SetRelativePosition(shapeComp->GetRelativePosition() + Float3{ 0.0f, 0.1f, 0.0f });
		actor->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
		actor->RootComponent->UpdateWorldTransform();
		//shapeComp->rigidBody->ClearRotation();
		shapeComp->rigidBody->SetPhysicalMaterial(targetState.physMaterial);
		shapeComp->SetShape(targetState.shape);
	}

	//new: input comp:
	if (actor->InputComponent) {
		actor->InputComponent->BindBehavior(targetState.inputCb);
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

	rb->bFastStable = false;
	rb->linearDamping = 1.0f;
	rb->angularDamping = 1.0f;

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
	//
	//this->InputComponent->BindBehavior([this](float delta) {
	//	this->MetalStateBehavior(delta);
	//	});

	//
	auto overlapCb = [=](AActor* other) {
		std::cout << "player: overlap with tag: " << other->tag << '\n';
		};

	this->shapeComponent->onOverlap.Add(overlapCb);


	{
		FFormState normalForm = {
	.mesh = CreateShared<CubeMesh>(),
	.material = Materials::GetIron(), //CreateShared<UMaterial>(),
	.shape = Box({ 1.0f ,1.0f, 1.0f }),
	.physMaterial = PhysicalMaterial{ 0.0f, 0.0f }
		};

		normalForm.inputCb = [this](float delta) {
			this->NormalStateBehavior(delta);
			};

		playerForms[EPlayerForm::Normal] = normalForm;
	}


	//
	{
		FFormState sphereForm = {
.mesh = CreateShared<SphereMesh>(),
.material = Materials::GetRustyIron(), //CreateShared<UMaterial>(),
.shape = Sphere{ 1.0f },
.physMaterial = PhysicalMaterial{ 0.0f, 0.8f },
		};

		sphereForm.inputCb = [this](float delta) {
			this->MetalStateBehavior(delta);
			};

		playerForms[EPlayerForm::MetalBall] = sphereForm;
	}


	{
		FFormState cubeForm = {
	.mesh = CreateShared<CubeMesh>(),
	.material = Materials::GetIceSurface(), //CreateShared<UMaterial>(),
	.shape = Box({ 1.0f ,1.0f, 1.0f }),
	.physMaterial = PhysicalMaterial{ 0.0f, 0.0f }
		};
		 
		cubeForm.inputCb = [this](float delta) {
			this->IceStateBehavior(delta);
			};

		playerForms[EPlayerForm::IceCube] = cubeForm;
	}


	TransitPlayerState(this, playerForms.at(EPlayerForm::MetalBall));

}



void APlayer::EndPlay()
{
	APawn::EndPlay();
}

void APlayer::OnTick(float delta)
{
	APawn::OnTick(delta);

	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (inputSystem->IsKeyJustPressed(KeyCode::Num1))
	{
		TransitPlayerState(this, playerForms.at(EPlayerForm::MetalBall));
	}
	else if (inputSystem->IsKeyJustPressed(KeyCode::Num2)) {
		TransitPlayerState(this, playerForms.at(EPlayerForm::IceCube));
	}
	 
	this->UploadPlayerState();

}

void APlayer::UploadPlayerState()
{
	playerState.speed = Length(this->shapeComponent->rigidBody->linearVelocity);

	auto gameState = GetWorld()->GetGameState<AGameState>();
	gameState->SetPlayerState(this->playerState);

}

void APlayer::NormalStateBehavior(float delta)
{
	 
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (inputSystem->IsKeyJustPressed(KeyCode::Space)) {
		shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 500.0f, 0.0f) * delta); 
	} 

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta); 
}

void APlayer::MetalStateBehavior(float delta)
{
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);

}

void APlayer::IceStateBehavior(float delta)
{
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);
}