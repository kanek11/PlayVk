#include "PCH.h"
#include "Item.h"

#include "Gameplay/World.h"
#include "GameState.h"

ABoxItem::ABoxItem() :AStaticMeshActor()
{ 
	this->tag = "item";

	this->shapeComponent->SetIsTrigger(true);
	this->shapeComponent->SetSimulatePhysics(false);

	Mesh::SetBox(this, Float3{ 1.0f,1.0f,1.0f });
}

void ABoxItem::BeginPlay()
{
	AStaticMeshActor::BeginPlay();
	 
	//this->staticMeshComponent->SetMaterial(Materials::GetDebugMesh());

	//
	auto overlapCb = [=](AActor*) {
		std::cout << "item: callback overlap\n";
		this->shapeComponent->SetCollisionEnabled(false);
		this->staticMeshComponent->SetVisible(false);
		};

	this->shapeComponent->onOverlap.Add(overlapCb);
}

void ABoxItem::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta); 

	float angle = MMath::ToRadians(delta * 100);
	
	auto targetRot = DirectX::XMQuaternionMultiply(RootComponent->GetRelativeRotation(), DirectX::XMQuaternionRotationRollPitchYaw(0.0f, angle,  0.0f));
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
}

void ATriggerVolume::OnTick(float delta)
{
	AStaticMeshActor::OnTick(delta);

	auto gameState = GetWorld()->GetGameState<AGameState>(); 

}
