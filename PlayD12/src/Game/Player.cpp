#include "PCH.h"
#include "Player.h"

#include "Application.h" 

#include "GameState.h"
#include "Item.h"

#include "Math/MMath.h"
#include "Math/AnimUtils.h"

#include "Controller.h"

constexpr float JUMP_SPEED = 6.0f;

constexpr float NORMAL_MAXSPEED = 15.0f;
constexpr float NORMAL_RATE_X = 10.0f;
constexpr float NORMAL_RATE_Z = 5.0f;
 
constexpr float METAL_RATE_X = 5.0f;
constexpr float METAL_RATE_Z = 5.0f;  


static bool ActorIsNotTrigger(AActor* actor) {

	if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
		if(!shapeComp->IsTrigger())  return true;
	}
	return false;
}


void EmissionFadeIn(UMaterial* mat, float nt) {
	mat->materialCB.useEmissiveMap = false;
	mat->materialCB.emissiveColor = Color::White.xyz();

	mat->materialCB.emissiveStrength = Anim::Easing::QuadInOut(nt);//Anim::Lerp(0.0f, 1.0f, nt);
}

void MaterialFadeOut(UMaterial* mat, float nt) {

	mat->materialCB.useEmissiveMap = false;
	mat->materialCB.emissiveColor = Color::White.xyz();

	mat->materialCB.emissiveStrength = Anim::Easing::QuadInOut(1.0f - nt);//Anim::Lerp(0.0f, 1.0f, nt);
}



void APlayer::DuringFormTransition(float nt, float startSize)
{
	//
	float r = Anim::Lerp(startSize, playerOGR * 1.00f, nt);
	//std::cout << "start:" << startSize << ", r" << r << '\n';
	Sphere dummyShape = { r };


	if (auto meshComp = this->GetComponent<UStaticMeshComponent>()) {
		//meshComp->SetMesh(targetState.mesh);
		//meshComp->SetMaterial(targetState.material);
		//meshComp->SetRelativeScale(scale);
	}

	if (auto shapeComp = this->GetComponent<UShapeComponent>()) {
		//this->RootComponent->SetRelativePosition(shapeComp->GetRelativePosition() + Float3{ 0.0f, 0.01f, 0.0f });
		//this->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
		//this->RootComponent->UpdateWorldTransform();
		////shapeComp->rigidBody->ClearRotation();
		//shapeComp->rigidBody->SetPhysicalMaterial(targetState.physMaterial);

		//shapeComp->rigidBody->linearDamping = 0.5f;
		//shapeComp->rigidBody->angularDamping = targetState.angularDamping;

		////new: set mass before reset shape;  for correct inertia
		//shapeComp->rigidBody->SetMass(targetState.mass);

		shapeComp->rigidBody->compliance = 0.0001f;

		shapeComp->SetColliderShape(dummyShape);
	}

}



void APlayer::RequestTransitForm(EPlayerForm targetForm)
{
	if (currForm == targetForm) return; 
	if (this->bDisableTransition) return;

	assert(playerForms.contains(targetForm));
	auto& targetState = playerForms.at(targetForm);

	this->bDisableTransition = true;

	auto tween = Anim::WaitFor(emitDuration, [=] {
		this->TransitForm(targetForm);
		this->bDisableTransition = false;
		});

	Float3 scale{};
	std::visit(overloaded{
	[this, &scale](Sphere& s) {
			 scale = this->staticMeshComponent->GetRelativeScale();
		},
	[this, &scale](Box& s) {
			 scale = this->staticMeshComponent->GetRelativeScale();
		},
		}, this->shapeComponent->shape);

	float startSize = scale.x();


	auto& mat = this->staticMeshComponent->GetMaterial();

	tween->onApply = [=](float nt) {
		if (playerForms.contains(currForm)) {
			auto& ogState = playerForms.at(currForm);
			if (ogState.duringTran) ogState.duringTran(nt);
		}

		EmissionFadeIn(mat.get(), nt);
		this->DuringFormTransition(nt, startSize);
		};
}

void APlayer::TransitForm(EPlayerForm targetForm)
{
	//todo init undefined state is not 
	if (playerForms.contains(currForm)) {
		auto& ogState = playerForms.at(currForm);
		if (ogState.onExit) ogState.onExit();
	}

	currForm = targetForm;

	assert(playerForms.contains(targetForm));
	auto& targetState = playerForms.at(targetForm);


	//====================
	auto& mat = this->staticMeshComponent->GetMaterial();
	//
	auto tween = Anim::WaitFor(0.3f, [=, &targetState] {
		if (targetState.onEnterTranMat) targetState.onEnterTranMat();
		});

	tween->onApply = [=](float nt) {
		MaterialFadeOut(mat.get(), nt);
		};
	//====================

	if (targetState.onEnter) targetState.onEnter();

	//========


	Float3 scale{};
	std::visit(overloaded{
	[this, &scale](Sphere& s) {
			//Mesh::SetSphere(this, s.radius); 
			scale = Float3(s.radius, s.radius, s.radius);
		},
	[this, &scale](Box& s) {
			scale = Float3{ s.halfExtents.x(), s.halfExtents.y(), s.halfExtents.z()};
		},
		}, targetState.shape);


	if (auto meshComp = this->GetComponent<UStaticMeshComponent>()) {
		meshComp->SetMesh(targetState.mesh);
		meshComp->SetMaterial(targetState.material);
		meshComp->SetRelativeScale(scale);
	}

	if (auto shapeComp = this->GetComponent<UShapeComponent>()) {
		//this->RootComponent->SetRelativePosition(shapeComp->GetRelativePosition() + Float3{ 0.0f, 0.01f, 0.0f });
		//this->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
		//this->RootComponent->UpdateWorldTransform();
		//shapeComp->rigidBody->ClearRotation();
		shapeComp->rigidBody->SetPhysicalMaterial(targetState.physMaterial);

		shapeComp->rigidBody->linearDamping = targetState.linearDamping;
		shapeComp->rigidBody->angularDamping = targetState.angularDamping;
		shapeComp->rigidBody->compliance = targetState.compliance;

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

	//new:
	auto& collider = shapeComponent->collider;
	collider->bNeedsEvent = true;

	this->RootComponent = this->shapeComponent;
	//this->shapeComponent->AttachTo(RootComponent.get());
	this->staticMeshComponent->AttachTo(RootComponent.get());

	this->SpringArmComponent->AttachTo(RootComponent.get());
	this->CameraComponent->AttachTo(SpringArmComponent.get());


	//todo:hardcoded for now
	//RootComponent->SetRelativeRotation(DirectX::XMQuaternionRotationRollPitchYaw(0, MMath::ToRadians(90.0f), 0)); 
	//std::cout << "curr rotation:" << ToString(RootComponent->GetWorldRotation()) << '\n';

	//update the world transform manually:
	RootComponent->UpdateWorldTransform();

}

void APlayer::BeginPlay()
{
	this->RegisterPhysicsHooks();

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

void APlayer::OnRegister()
{
	APawn::OnRegister();
	//bug fix: you don't modify actor list on being iterated, so we must modifiy the logic;
	//this could be...antipattern

	{
		FFormState form{};
		form.mesh = CreateShared<SphereMesh>();
		form.material = Materials::AttachPlayer(Materials::GetPlayerMat());
		form.shape = Sphere{ playerOGR };
		form.physMaterial = PhysicalMaterial{ 0.0f, 0.4f };

		form.compliance = 0.0001f;
		//FFormState form{};
		//form.mesh = CreateShared<CubeMesh>();
		//form.material = Materials::GetIron(); //CreateShared<UMaterial>(),
		//form.shape = Box({ 1.0f ,1.0f, 1.0f });
		//form.physMaterial = PhysicalMaterial{ 0.0f, 2.0f };

		form.inputCb = [this](float delta) {
			this->NormalStateBehavior(delta);
			};


		form.onEnterTranMat = [this]() {

			auto& mat = this->playerForms[EPlayerForm::Normal].material;
			mat->materialCB.useEmissiveMap = true;
			mat->materialCB.emissiveStrength = 1.0f;

			};


		playerForms[EPlayerForm::Normal] = form;

	}


	//clone
	{
		auto& mat = this->playerForms[EPlayerForm::Normal].material;

		FFormState form{};
		form.mesh = CreateShared<SphereMesh>();
		form.material = mat; //Materials::AttachPlayer(Materials::GetPlayerMat());
		form.shape = Sphere{ playerShrinkR };
		form.physMaterial = PhysicalMaterial{ 0.0f, 0.4f };

		form.compliance = 0.00001f;

		form.inputCb = [this](float delta) {
			this->CloneStateBehavior(delta);
			};

		form.onEnter = [this]() {
			this->ShowParticles();
			};

		form.onExit = [this]() {
			this->HideParticles();
			};

		form.duringTran = [this](float nt) {
			this->MoveParticles(nt);
			};

		form.onEnterTranMat = [this]() {
			auto& mat = this->playerForms[EPlayerForm::Clone].material;
			mat->materialCB.useEmissiveMap = true;
			mat->materialCB.emissiveStrength = 1.0f;
			};

		playerForms[EPlayerForm::Clone] = form;
	}

	//
	{
		FFormState form{};
		form.mesh = CreateShared<SphereMesh>();
		form.material = Materials::GetRustyIron();
		form.shape = Sphere{ playerOGR };
		form.physMaterial = PhysicalMaterial{ 0.0f, 1.0f };

		form.compliance = 0.000001f;

		form.inputCb = [this](float delta) {
			this->MetalStateBehavior(delta);
			};

		form.mass = 10.0f;

		playerForms[EPlayerForm::MetalBall] = form;
	}


	//ice
	{
		auto size = playerOGR / 1.3f;
		FFormState form{};
		form.mesh = CreateShared<CubeMesh>();
		form.material = Materials::GetIceSurface();
		form.shape = Box({ size ,size, size });
		form.physMaterial = PhysicalMaterial{ 0.0f, 0.0f };
		form.mass = 2.0f;
		form.angularDamping = 0.98f;
		form.compliance = 0.0001f;

		form.inputCb = [this](float delta) {
			this->IceStateBehavior(delta);
			};

		form.onEnter = [this]() {
			this->RootComponent->SetRelativeRotation(DirectX::XMQuaternionIdentity());
			this->RootComponent->UpdateWorldTransform();
			};

		playerForms[EPlayerForm::IceCube] = form;
	}

	//immeidate
	TransitForm(EPlayerForm::Normal);



	//todo: enum enumerator?  
	abilities[EPlayerForm::Normal] = FAbilityRuntime{};
	abilities[EPlayerForm::IceCube] = FAbilityRuntime{};
	abilities[EPlayerForm::MetalBall] = FAbilityRuntime{};
	abilities[EPlayerForm::Clone] = FAbilityRuntime{};


	//needed valid tex
	this->SpawnParticles();

	//this->ShowParticles();
}

void APlayer::UploadPlayerState()
{
	//=========== 
	auto gameState = GetWorld()->GetGameState<AGameState>();
	gameState->SetPlayerState(this->playerState); 

	EDGrounded.Update(bGroundedThisFrame);
	if (EDGrounded.risingEdge) {
		std::cout << "ground event" << '\n';

		if(gameState->startPlaying)
		Anim::VibrateFor(0.3f, 0.3f, 0.2f);

	}

	else if (EDGrounded.fallingEdge) {

		std::cout << "jump event" << '\n';
	}

	//============
	playerState.speed = Length(shapeComponent->rigidBody->linearVelocity);

	playerState.accel = this->shapeComponent->rigidBody->linearAccel;

	playerState.currForm = this->currForm;

	playerState.abilitiesRT = abilities;  

	auto& pos = this->RootComponent->GetWorldPosition();
	//std::cout << "player pos:" << ToString(pos) << '\n';
	if (!bToReset && !MMath::PointInAABB2D({-10.1f, 10.1f, 0.0f, MMath::FLOAT_MAX }, pos.x(), pos.z()))
	{
		std::cout << "player out of bound, try respawn" << '\n';
		//Anim::WaitFor(1.0f, [=]() {
		//	gameState->shouldRespawn = true;
		//	bToReset = true;
		//	});
		gameState->shouldRespawn = true;
		bToReset = true;
	}

}



void APlayer::TickPlayingPersistent(float delta)
{
	auto gameState = GetWorld()->GetGameState<AGameState>(); 
	if (!gameState->startPlaying)  return;

	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (!this->bDisableTransition) {

		//
		if (inputSystem->GetAction(EAction::Normal))
		{ 
			RequestTransitForm(EPlayerForm::Normal);
		}
		else if (inputSystem->GetAction(EAction::Metal))
		{
			auto form = EPlayerForm::MetalBall;
			auto& ability = abilities[form]; 
			if (ability.remaining > 1e-6f)
			{
				RequestTransitForm(form);
			}

			
		}
		else if (inputSystem->GetAction(EAction::Ice))
		{
			auto form = EPlayerForm::IceCube;
			auto& ability = abilities[form];
			if (ability.remaining > 1e-6f)
			{
				RequestTransitForm(form);
			}
		}
		else if (inputSystem->GetAction(EAction::Clone))
		{
			auto form = EPlayerForm::Clone;
			auto& ability = abilities[form];
			if (ability.remaining > 1e-6f)
			{
				RequestTransitForm(form);
			}
		}


	}


	//std::cout << "isGrounded ?" << this->bGroundedThisFrame << '\n';

}

void APlayer::NormalStateBehavior(float delta)
{
	auto& form = playerForms[EPlayerForm::Normal];
	//
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (bGroundedThisFrame)
	{
		if (inputSystem->IsKeyJustPressed(KeyCode::Space)) {
			shapeComponent->rigidBody->ApplyImpulse(Float3(0.0f, JUMP_SPEED, 0.0f));
		}

		if (Length(shapeComponent->rigidBody->linearVelocity) <= NORMAL_MAXSPEED) {
			float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
			float axisX = inputSystem->GetAxis(EAxis::MoveX);

			//normalized direction:
			if (LengthSq(Float3(axisX, 0.0f, axisZ)) < 1e-6f) return;
			auto dir3D = MMath::Normalize(Float3(axisX, 0.0f, axisZ));

			shapeComponent->rigidBody->ApplyForceRate(dir3D * NORMAL_RATE_Z * form.mass * delta);

		}
		else {
			float axisX = inputSystem->GetAxis(EAxis::MoveX);
			shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * NORMAL_RATE_X * form.mass, 0.0f, 0.0f) * delta);
		}


	}
	else {

		float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
		shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 1.0f * form.mass) * delta);

		float axisX = inputSystem->GetAxis(EAxis::MoveX);
		shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 2.0f * form.mass, 0.0f, 0.0f) * delta);

	}

}


void APlayer::CloneStateBehavior(float delta)
{
	auto& form = playerForms[EPlayerForm::Clone];
	auto& ability = abilities[EPlayerForm::Clone];
	ability.remaining -= delta;
	if (ability.remaining < 0.0f)
	{
		ability.remaining = 0.0f;
		RequestTransitForm(EPlayerForm::Normal);
	}

	//
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	if (bGroundedThisFrame)
	{
		if (inputSystem->IsKeyJustPressed(KeyCode::Space)) {
			shapeComponent->rigidBody->ApplyImpulse(Float3(0.0f, JUMP_SPEED, 0.0f));
		}

		if (Length(shapeComponent->rigidBody->linearVelocity) <= NORMAL_MAXSPEED) {
			float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
			float axisX = inputSystem->GetAxis(EAxis::MoveX);

			//normalized direction:
			if (LengthSq(Float3(axisX, 0.0f, axisZ)) < 1e-6f) return;
			auto dir3D = MMath::Normalize(Float3(axisX, 0.0f, axisZ));

			shapeComponent->rigidBody->ApplyForceRate(dir3D * NORMAL_RATE_Z * form.mass * delta);
			
		}
		else {
			float axisX = inputSystem->GetAxis(EAxis::MoveX);
			shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * NORMAL_RATE_X * form.mass, 0.0f, 0.0f) * delta);
		}


	}
	else {

		float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
		shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * 1.0f * form.mass) * delta);

		float axisX = inputSystem->GetAxis(EAxis::MoveX);
		shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * 2.0f * form.mass, 0.0f, 0.0f) * delta);

	}

}


void APlayer::MetalStateBehavior(float delta)
{
	auto& form = playerForms[EPlayerForm::MetalBall];


	auto& ability = abilities[EPlayerForm::MetalBall];
	ability.remaining -= delta;
	if (ability.remaining < 0.0f)
	{
		ability.remaining = 0.0f;
		RequestTransitForm(EPlayerForm::Normal);
	}

	//
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
	shapeComponent->rigidBody->ApplyForceRate(Float3(0.0f, 0.0f, axisZ * METAL_RATE_Z * form.mass) * delta);

	float axisX = inputSystem->GetAxis(EAxis::MoveX);
	shapeComponent->rigidBody->ApplyForceRate(Float3(axisX * METAL_RATE_X * form.mass, 0.0f, 0.0f) * delta);

}

void APlayer::IceStateBehavior(float delta)
{
	auto& form = playerForms[EPlayerForm::IceCube];

	auto& ability = abilities[EPlayerForm::IceCube];
	ability.remaining -= delta;
	if (ability.remaining < 0.0f)
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

		itemActor->bConsumed = true;
	}

}


void APlayer::AddPayloadImmediate(const FAbilityPayload& payload)
{
	if (!playerForms.contains(payload.formType))  
		return;

	auto& abilityRT = abilities[payload.formType];
	abilityRT.remaining += payload.duration;
}

void APlayer::RegisterPhysicsHooks()
{
	 
	auto prePhysicsCb = [=]() {
		this->bGroundedThisFrame = false;
		//std::cout << "pre physics called" << '\n';
		};

	this->shapeComponent->onPrePhysicsEvents.Add(prePhysicsCb);

	//
	auto overlapCb = [=](AActor* other, Contact contact) {
		auto gameState = GetWorld()->GetGameState<AGameState>();
		gameState->SetPlayerState(this->playerState);

		if (other->tag == "item") {

			std::cout << "player: overlap with tag: " << other->tag << '\n';
			this->OnOverlapItem(other);
		}  
		else if (other->tag == "floor") {
			
			bGroundedThisFrame = true;
		}

		else if (ActorIsNotTrigger(other)) {
			auto scale = MMath::Normalize01(contact.penetration, 0.0f, 0.2f, 0.005f) * 1.0f; 
			scale *= this->shapeComponent->rigidBody->mass;

			if (scale > 0.02f) {
				//std::cout << "contact pen:" << contact.penetration << ",scale:" << scale << '\n'; 
				scale = std::clamp(scale, 0.0f, 1.0f);
				Anim::VibrateFor(scale, scale, 0.2f);
			}
		}

		};

	this->shapeComponent->onOverlap.Add(overlapCb);


}

void APlayer::SpawnParticles()
{
	auto& form = playerForms[EPlayerForm::Normal];

	particlePos = MMath::GenSpherePattern(playerShrinkR, playerOGR, particleR, 0.35f);

	auto& owningLevel = this->level;

	for (auto& pos : particlePos) {
		auto actor = CreateActor<AStaticMeshActor>();

		auto& rb = actor->shapeComponent->rigidBody;
		rb->simulatePhysics = false;
		rb->simulateRotation = false;
		//actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

		actor->staticMeshComponent->SetVisible(false);
		actor->staticMeshComponent->SetMaterial(form.material);

		actor->shapeComponent->SetCollisionEnabled(false);




		actor->shapeComponent->rigidBody->SetPhysicalMaterial(PhysicalMaterial{ 0.0f, 10.0f });
		actor->shapeComponent->rigidBody->SetMass(form.mass / 10.0f);
		actor->shapeComponent->rigidBody->linearDamping = form.linearDamping;
		actor->shapeComponent->rigidBody->angularDamping = form.angularDamping;

		this->particleActors.push_back(actor);

		Mesh::SetSphere(actor.get(), particleR);
		owningLevel->AddActor(actor);
	}


}

void APlayer::ShowParticles()
{
	auto& form = playerForms[EPlayerForm::Normal];
	auto superPosition = RootComponent->GetWorldPosition();

	int index{ 0 };
	for (auto& actor : particleActors) {

		assert(particlePos.size() > index);
		auto& initOffset = particlePos[index];
		actor->RootComponent->SetRelativePosition(superPosition + initOffset);
		actor->RootComponent->UpdateWorldTransform();

		auto& rb = actor->shapeComponent->rigidBody;
		rb->simulatePhysics = true;
		rb->simulateRotation = true;

		rb->linearVelocity = this->shapeComponent->rigidBody->linearVelocity;

		actor->staticMeshComponent->SetVisible(true);
		actor->staticMeshComponent->SetMaterial(form.material);
		actor->shapeComponent->SetCollisionEnabled(true);

		index++;
	}
}

void APlayer::HideParticles()
{
	for (auto& actor : particleActors) {

		actor->staticMeshComponent->SetVisible(false);
		actor->shapeComponent->SetCollisionEnabled(false);

		auto& rb = actor->shapeComponent->rigidBody;
		rb->simulatePhysics = false;
		rb->simulateRotation = false;
	}
}

void APlayer::MoveParticles(float nt)
{
	auto& form = playerForms[EPlayerForm::Normal];
	auto superPosition = RootComponent->GetWorldPosition();
	int index{ 0 };
	for (auto& actor : particleActors) {

		assert(particlePos.size() > index);
		auto& initOffset = particlePos[index];
		auto targetPos = superPosition + initOffset;

		actor->shapeComponent->SetCollisionEnabled(false);

		auto& rb = actor->shapeComponent->rigidBody;
		rb->simulateRotation = false;

		auto currPos = actor->RootComponent->GetRelativePosition();
		actor->RootComponent->SetRelativePosition(Anim::Lerp(currPos, targetPos, nt));
		actor->RootComponent->UpdateWorldTransform();

		index++;
	}





}