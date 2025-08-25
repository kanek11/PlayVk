#include "PCH.h"
#include "Item.h"

#include "Gameplay/World.h"
#include "GameState.h"

AItem::AItem() :AStaticMeshActor()
{
	this->tag = "item";

	this->shapeComponent->SetIsTrigger(true);
	this->shapeComponent->SetSimulatePhysics(false);

	Mesh::SetBox(this, Float3{ 1.0f,1.0f,1.0f });
}

void AItem::BeginPlay()
{
	AStaticMeshActor::BeginPlay();

	//this->staticMeshComponent->SetMaterial(Materials::GetDebugMesh());

	//
	auto overlapCb = [=](AActor*, Contact) {
		std::cout << "item: callback overlap\n";
		this->shapeComponent->SetCollisionEnabled(false);

		auto tween = Anim::WaitFor(0.2f, [=]() {
			this->staticMeshComponent->SetVisible(false);
			});
		tween->onApply = [=](float nt) { 
			auto mat = this->staticMeshComponent->GetMaterial();
			mat->materialCB.useEmissiveMap = false;
			mat->materialCB.emissiveColor = Color::Yellow.xyz(); 
			mat->materialCB.emissiveStrength = Anim::Easing::QuadInOut(nt); 

			};

		//game thread flag is more predictable than physics?
		//this->bConsumed = true;
		};

	this->shapeComponent->onOverlap.Add(overlapCb);
}

void AItem::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta);

	float angle = MMath::ToRadians(delta * 100);

	auto targetRot = DirectX::XMQuaternionMultiply(RootComponent->GetRelativeRotation(), DirectX::XMQuaternionRotationRollPitchYaw(0.0f, angle, 0.0f));
	RootComponent->SetRelativeRotation(targetRot);

}

ATriggerVolume::ATriggerVolume() :AStaticMeshActor()
{
	this->tag = "trigger";

	this->shapeComponent->SetIsTrigger(true);
	this->shapeComponent->SetSimulatePhysics(false);

	Mesh::SetBox(this, Float3{ 1.0f,1.0f,1.0f });

	this->staticMeshComponent->SetMaterial(Materials::GetTransparentMesh());

}

void ATriggerVolume::BeginPlay()
{
	AStaticMeshActor::BeginPlay();


	auto gameState = GetWorld()->GetGameState<AGameState>();
	this->shapeComponent->onOverlap.Add([=](AActor* other, Contact contact) {
		if (other->tag == "player") {
			gameState->onGoal.BlockingBroadCast();
			this->shapeComponent->SetCollisionEnabled(false);

		}
		});
}

void ATriggerVolume::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta);

	//auto gameState = GetWorld()->GetGameState<AGameState>(); 
	//gameState->onGoal.BlockingBroadCast();

}

//---------
AIceItem::AIceItem() : AItem()
{
	auto mat = Materials::GetIceSurface();
	//mat->materialCB.emissiveColor = Color::Yellow.xyz();
	//mat->materialCB.emissiveStrength = 0.1f;

	this->staticMeshComponent->SetMaterial(mat);

	payload.formType = EPlayerForm::IceCube;
	payload.duration = 1.0f;
}


//===============

AMetalItem::AMetalItem()
{
	auto mat = Materials::GetRustyIron();
	//mat->materialCB.emissiveColor = Color::Yellow.xyz();

	this->staticMeshComponent->SetMaterial(mat);

	Mesh::SetSphere(this, 0.5f);

	payload.formType = EPlayerForm::MetalBall;
	payload.duration = 1.0f;
}


//===============
ACloneItem::ACloneItem()
{
	auto mat = Materials::GetPlayerMat();
	//mat->materialCB.emissiveColor = Color::Yellow.xyz();

	this->staticMeshComponent->SetMaterial(mat); 

	Mesh::SetSphere(this, 0.5f);

	payload.formType = EPlayerForm::Clone;
	payload.duration = 1.0f;
}



//----
//void URotateComponent::TickComponent(float delta)
//{
//	auto RootComponent = this->GetOwner().lock()->RootComponent;
//	auto targetRot = DirectX::XMQuaternionMultiply(RootComponent->GetRelativeRotation(), DirectX::XMQuaternionRotationRollPitchYaw(0.0f, MMath::ToRadians(rotationSpeed * delta), 0.0f));
//	RootComponent->SetRelativeRotation(targetRot);
//}


ARotateBox::ARotateBox() : AStaticMeshActor()
{
	this->tag = "env";
	this->shapeComponent->SetSimulatePhysics(false);
	Mesh::SetBox(this, Float3{ 1.0f,1.0f,1.0f });
}
void ARotateBox::BeginPlay()
{
}

void ARotateBox::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta);
	auto targetRot = DirectX::XMQuaternionMultiply(RootComponent->GetRelativeRotation(), DirectX::XMQuaternionRotationRollPitchYaw(0.0f, MMath::ToRadians(rotationSpeed * delta), 0.0f));
	RootComponent->SetRelativeRotation(targetRot);
}


//void UOscillateComponent::TickComponent(float delta)
//{
//	phase += MMath::ToRadians(delta);
//	auto offset = std::sin(phase) * mag;
//	
//	auto targetPos = basePos + Float3(0.0f, offset, 0.0f);
//	//RootComponent->SetRelativeRotate(targetRot);
//}

AOscillateBox::AOscillateBox() : AStaticMeshActor()
{
	this->tag = "env";
	this->shapeComponent->SetSimulatePhysics(false);
	Mesh::SetBox(this, Float3{ 1.0f,1.0f,1.0f });
}

void AOscillateBox::BeginPlay()
{
	AStaticMeshActor::BeginPlay();

	if (RootComponent)
		m_startPos = RootComponent->GetWorldPosition();
}

void AOscillateBox::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta);

	m_elapsedTime += delta;
 
	float offsetY = m_amplitude * std::sin(m_frequency * m_elapsedTime + m_phase);

	if (RootComponent)
	{
		Float3 newPos = m_startPos + Float3(0.0f, offsetY, 0.0f);
		RootComponent->SetRelativePosition(newPos);
		RootComponent->UpdateWorldTransform();

		//std::cout << "oscillate to:" << ToString(newPos) << '\n';
	}
}
