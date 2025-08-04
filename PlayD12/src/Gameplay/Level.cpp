#include "PCH.h"
#include "Level.h"

#include "Application.h"
#include "Render/Renderer.h"

#include "Render/StaticMeshProxy.h"

#include <random> 

using namespace DirectX; 

namespace Gameplay {


void ULevel::OnLoad()
{

}

void ULevel::OnUpdate(float delta)
{
    for (auto& actor : m_actors) {
        actor->OnTick(delta);
    } 

    //new:
	UpdateTransforms();

    defaultCamera.Tick(delta);
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    renderer->SubmitCamera(defaultCamera);
}
 

void ULevel::AddActor(SharedPtr<AActor> actor)
{
    actor->level = this; 

    //this should come after the level registery;
    actor->RegisterAllComponents();   
    m_actors.push_back(actor); 
}


void ULevel::UpdateTransforms()
{
    for (auto& actor : m_actors) {

        if (actor->RootComponent) {
            actor->RootComponent->UpdateWorldTransform();
        }
    }
}

void ULevel::ForEachComponent(const sceneIterFn& fn)
{
    for (auto& actor : m_actors) {
        if (actor->RootComponent) {
            TraverseTree(actor->RootComponent.get(), fn);
        }
    }
}

void ULevel::TraverseTree(USceneComponent* comp, const sceneIterFn& fn)
{
    fn(comp);
    for (auto* child : comp->GetChildren()) {
        TraverseTree(child, fn);
    }
} 
 

}

