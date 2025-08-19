#include "PCH.h"
#include "Player.h"

#include "Application.h"

#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"
#include "Item.h"


void APlayer::RequestTransitForm(EPlayerForm targetForm)
{
	if (currForm == targetForm) return;
	currForm = targetForm;

	this->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());

	assert(playerForms.contains(targetForm));
	auto& targetState = playerForms.at(targetForm);

	if (auto meshComp = this->GetComponent<UStaticMeshComponent>()) {
		meshComp->SetMesh(targetState.mesh);
		meshComp->SetMaterial(targetState.material);
	}

	if (auto shapeComp = this->GetComponent<UShapeComponent>()) {
		this->RootComponent->SetRelativePosition(shapeComp->GetRelativePosition() + Float3{ 0.0f, 0.5f, 0.0f });
		this->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
		this->RootComponent->UpdateWorldTransform();
		//shapeComp->rigidBody->ClearRotation();
		shapeComp->rigidBody->SetPhysicalMaterial(targetState.physMaterial);

		//new: set mass before reset shape;  for correct inertia
		shapeComp->rigidBody->SetMass(targetState.mass);
		shapeComp->SetShape(targetState.shape);
	}

	//new: input comp:
	if (this->InputComponent) {
		this->InputComponent->BindBehavior(targetState.inputCb);
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
	auto overlapCb = [=](AActor* other) {
		std::cout << "player: overlap with tag: " << other->tag << '\n';
		if(other->tag == "item")
		this->OnOverlapItem(other);
		};

	this->shapeComponent->onOverlap.Add(overlapCb);


	{
		FFormState form{};
		form.mesh = CreateShared<SphereMesh>();
		form.material = Materials::GetSnowSurface(); //CreateShared<UMaterial>(),
		form.shape = Sphere{ 1.0f };
		form.physMaterial = PhysicalMaterial{ 0.0f, 0.4f }; 
		//FFormState form{};
		//form.mesh = CreateShared<CubeMesh>();
		//form.material = Materials::GetIron(); //CreateShared<UMaterial>(),
		//form.shape = Box({ 1.0f ,1.0f, 1.0f });
		//form.physMaterial = PhysicalMaterial{ 0.0f, 2.0f };

		form.inputCb = [this](float delta) {
			this->NormalStateBehavior(delta);
			};

		playerForms[EPlayerForm::Normal] = form;
	}


	//
	{
		FFormState form{};
		form.mesh = CreateShared<SphereMesh>();
		form.material = Materials::GetRustyIron(); //CreateShared<UMaterial>(),
		form.shape = Sphere{ 1.0f };
		form.physMaterial = PhysicalMaterial{ 0.0f, 1.0f }; 

		form.inputCb = [this](float delta) {
			this->MetalStateBehavior(delta);
			}; 
		
		form.mass = 10.0f;

		playerForms[EPlayerForm::MetalBall] = form;
	}


	{
		FFormState form{};
		form.mesh = CreateShared<CubeMesh>();
		form.material = Materials::GetIceSurface(); //CreateShared<UMaterial>(),
		form.shape = Box({ 1.0f ,1.0f, 1.0f });
		form.physMaterial = PhysicalMaterial{ 0.0f, 0.0f }; 

		form.inputCb = [this](float delta) {
			this->IceStateBehavior(delta);
			};

		playerForms[EPlayerForm::IceCube] = form;
	}


	RequestTransitForm(EPlayerForm::Normal);


	//todo: enum enumerator?

	abilities[EPlayerForm::Normal] = FAbilityRuntime{};
	abilities[EPlayerForm::IceCube] = FAbilityRuntime{};
	abilities[EPlayerForm::MetalBall] = FAbilityRuntime{};
}



void APlayer::EndPlay()
{
	APawn::EndPlay();
}

void APlayer::OnTick(float delta)
{
	APawn::OnTick(delta); 

	this->TickPlayingPersistent(delta);

	this->UploadPlayerState(); 

	//for (auto& [form, rt] : abilities) {
	//	std::cout << "form remain:" << rt.remaining << '\n';
	//}
}

void APlayer::UploadPlayerState()
{
	playerState.speed = this->shapeComponent->rigidBody->prevLinearSpeed; 

	playerState.accel = this->shapeComponent->rigidBody->linearAccel; 

	playerState.abilitiesRT = abilities; 

	auto gameState = GetWorld()->GetGameState<AGameState>();
	gameState->SetPlayerState(this->playerState);

}





void APlayer::TickPlayingPersistent(float delta)
{
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();



	//
	if (inputSystem->IsKeyJustPressed(KeyCode::Num1))
	{ 
		RequestTransitForm(EPlayerForm::Normal);
	} 

	else if (inputSystem->IsKeyJustPressed(KeyCode::Num2))
	{ 
		RequestTransitForm(EPlayerForm::MetalBall);
	}
	else if (inputSystem->IsKeyJustPressed(KeyCode::Num3)) {
		 
		RequestTransitForm(EPlayerForm::IceCube);
	}

}

void APlayer::NormalStateBehavior(float delta)
{ 
	//
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (inputSystem->IsKeyJustPressed(KeyCode::Space)) {
		shapeComponent->rigidBody->ApplyImpulse(Float3(0.0f, 4.0f, 0.0f) * delta);
	}
	
	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);


}

void APlayer::MetalStateBehavior(float delta)
{
	auto& ability = abilities[EPlayerForm::IceCube];
	ability.remaining -= delta;
	if (ability.remaining < 0.0f)
	{
		ability.remaining = 0.0f;
		RequestTransitForm(EPlayerForm::Normal);
	}

	//
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 10.0f) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 10.0f, 0.0f, 0.0f) * delta);

}

void APlayer::IceStateBehavior(float delta)
{
	auto& ability = abilities[EPlayerForm::IceCube];
	ability.remaining -= delta;
	if(ability.remaining < 0.0f)
	{
		ability.remaining = 0.0f; 
		RequestTransitForm(EPlayerForm::Normal);
	}


	//auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	//float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	//shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 5.0f) * delta);

	//float axisX = inputSystem->GetAxis(EAxis::MoveX);
	//shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 5.0f, 0.0f, 0.0f) * delta);
}

void APlayer::OnOverlapItem(AActor* item)
{
	std::cout << "detect overlap with item:" << '\n';

	if (auto itemActor = dynamic_cast<AItem*>(item)) {

		if (!itemActor->bConsumed) 
     	this->AddPayloadImmediate(itemActor->GetPlayload());
	}

}


void APlayer::AddPayloadImmediate(const FAbilityPayload& payload)
{
	if (!playerForms.contains(payload.formType))  return; 

	auto& abilityRT = abilities[payload.formType];
	abilityRT.remaining += payload.duration;

}
