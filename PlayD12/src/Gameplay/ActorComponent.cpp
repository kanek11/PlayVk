#include "PCH.h"

#include "ActorComponent.h"

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
    }

    WeakPtr<AActor> UActorComponent::GetOwner() const
    {
        return this->m_owner;
    }

}  