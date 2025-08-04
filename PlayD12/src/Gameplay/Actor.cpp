#pragma once
#include "PCH.h"

#include "Actor.h"

#include "Level.h"

namespace Gameplay
{

    void AActor::AddComponent(const SharedPtr<UActorComponent>& component)
    {
        m_components.push_back(component);
    }

    void AActor::RegisterAllComponents()
    {
        for (auto& component : m_components)
        {
            component->RegisterOwner(this->shared_from_this());
        }
    }

    void AActor::OnTick(float DeltaTime)
    {
        for (auto& component : m_components)
        {
            component->TickComponent(DeltaTime);
        }
    }

    void AActor::BeginPlay()
    {
        for (auto& component : m_components)
        {
        	component->BeginPlay();
        }
    }

    void AActor::EndPlay()
    {
        for (auto& component : m_components)
        {
        	component->EndPlay();
        }
    }

    UWorld* AActor::GetWorld() const
    {
        return level ? level->owningWorld : nullptr;
    }



} 