#include "PCH.h"
#include "MeshComponent.h"

#include "StaticMeshActor.h"

#include "Gameplay/World.h"

void Gameplay::UStaticMeshComponent::TickComponent(float delta)
{
    UMeshComponent::TickComponent(delta);

	//if mesh is not set, skip tick:
	assert(m_mesh != nullptr && "static mesh component mesh is not set");

    //new: register to scene:
    //todo: update proxy on dirty change;
    auto proxy = CreateSceneProxy();

    this->GetWorld()->scene.AddPrimitive(proxy);
}

void Gameplay::UStaticMeshComponent::OnRegister()
{
    UMeshComponent::OnRegister();
    std::cout << "static mesh on register\n";
}

FStaticMeshProxy Gameplay::UStaticMeshComponent::CreateSceneProxy()
{
    auto instanceData = DebugGenerateInstanceData();

    FStaticMeshProxy proxy = {
.modelMatrix = this->GetWorldTransform().ToMatrix(),
.mesh = m_mesh.get(),
.material = m_material.get(),
.instanceData = instanceData.data(),
.instanceCount = instanceData.size(),
    };
    return proxy;
}