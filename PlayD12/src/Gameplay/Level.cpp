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
        //auto dftPlayer = CreateActor<APawn>(); 
        auto dftPlayer = CreateActor<APlayer>();

        dftPlayer->RootComponent->SetRelativePosition({ 0.0f, 4.0f, -2.0f });
        dftPlayer->RootComponent->UpdateWorldTransform();

        //auto possess the default player 
        if (auto controller = this->owningWorld->GetFirstPlayerController(); controller != nullptr) {
            std::cout << "level: default controller possess default default player\n";
            controller->Possess(dftPlayer.get());
        } 

        this->AddActor(dftPlayer);
    }

    void ULevel::OnTick(float delta)
    {
        //todo: introduce late update?
        UpdateTransforms();
        //
        for (auto& actor : m_actors) {
            actor->OnTick(delta);
        }
        //
        for (auto& HUD : m_buttons) {
            HUD->Tick(delta);
        }

        //defaultCamera.Tick(delta);
        //auto renderer = GameApplication::GetInstance()->GetRenderer();
        //renderer->SubmitCamera(defaultCamera);
    }

    void ULevel::BeginPlay()
    {
        for (auto& actor : m_actors) {
            actor->BeginPlay();
        }
    }

    void ULevel::EndPlay()
    {
        for (auto& actor : m_actors) {
            actor->EndPlay();
        }
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