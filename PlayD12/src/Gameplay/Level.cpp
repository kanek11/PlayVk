#include "PCH.h"
#include "Level.h"

#include "Application.h"
#include "Render/Renderer.h"

#include "Render/StaticMeshProxy.h"

#include "Game/Player.h"


using namespace DirectX;

namespace Gameplay {

    void ULevel::OnLoad()
    {

    }

    void ULevel::OnUnload()
    {
        for (auto& actor : actors) {
            actor.reset();
        }
        actors.clear();
    }

    void ULevel::OnTick(float delta)
    {
        //todo: introduce late update?
        UpdateTransforms();

        //
        for (auto& actor : actors) {
            actor->OnTick(delta);
        }

        //std::cout << "curr actor num: " << actors.size() << '\n';
        //
        //for (auto& HUD : m_buttons) {
        //    HUD->Tick(delta);
        //}

        //defaultCamera.Tick(delta);
        //auto renderer = GameApplication::GetInstance()->GetRenderer();
        //renderer->SubmitCamera(defaultCamera);
    }

    void ULevel::RouteActorBeginPlay()
    {
        for (auto& actor : actors) {
            actor->BeginPlay();
        }
    }

    void ULevel::RouteActorEndPlay()
    {
        for (auto& actor : actors) {
            actor->EndPlay();
        }
    }


    void ULevel::AddActor(SharedPtr<AActor> actor)
    {
        actor->level = this;

        //this should come after the level registery;
        actor->OnRegister();
        actors.push_back(actor);
    }

    void ULevel::AddActorPending(SharedPtr<AActor> actor)
    {
        actor->level = this;

        //this should come after the level registery;
        //actor->RegisterAllComponents();
        //pendingActors.push_back(actor);
    }


    void ULevel::UpdateTransforms()
    {
        for (auto& actor : actors) { 
            if (actor->RootComponent) {
                actor->RootComponent->UpdateWorldTransform();
            }
        }
    }

    void ULevel::ForEachComponent(const sceneIterFn& fn)
    {
        for (auto& actor : actors) {
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