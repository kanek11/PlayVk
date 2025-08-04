#include "PCH.h"

#include "ActorComponent.h"

#include "Actor.h"

namespace Gameplay
{
    UActorComponent::UActorComponent() :
        UObject()
    {
    }

    void UActorComponent::TickComponent(float DeltaTime)
    {
        // do nothing for now;
    }

    void UActorComponent::InitializeComponent()
    {
    }

    void UActorComponent::BeginPlay()
    {
    }

    void UActorComponent::EndPlay()
    {
    }

    void UActorComponent::RegisterOwner(const SharedPtr<AActor>& owner)
    {
        this->m_owner = owner;

        //new:
        this->OnRegister();
    }

    WeakPtr<AActor> UActorComponent::GetOwner() const
    {
        return this->m_owner;
    }

    UWorld* UActorComponent::GetWorld() const
    {
        return m_owner.expired() ? nullptr : m_owner.lock()->GetWorld();
    }
    void UActorComponent::OnRegister()
    {
    }


}  