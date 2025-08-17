#pragma once 
#include "PCH.h"
#include "Object.h"
#include "ActorComponent.h"
#include "SceneComponent.h"


// todo: actor can be placed in "level, world",
// the load/streaming of actors can be subtle;

// std::enable_shared_from_this is safer ;

using ActorHandle = uint32_t;

namespace Gameplay
{
    class ULevel;
    class UWorld;

    // Actor can be placed in the game world;
    //<<abstract>>
    class AActor : public UObject, public std::enable_shared_from_this<AActor>
    {
    public:
        virtual ~AActor() = default;
        AActor() = default;

    public:
        // game loop
        virtual void OnTick(float DeltaTime);
        virtual void BeginPlay();
        virtual void EndPlay();

    public:
        // dynamic components
        void AddComponent(const SharedPtr<UActorComponent>& component);
        // void RemoveComponent();

        void RegisterAllComponents();

        // factory for subobjects;  otherwise the component must be register manually;
        template <typename T>
        SharedPtr<T> CreateComponentAsSubObject()
        {
            auto _component = CreateActorComponent<T>();
            this->AddComponent(_component);

            return _component;
        }

        template <typename T>
        SharedPtr<T> GetComponent() const // dynamic cast is a bit slow but safe
        {
            for (auto& component : m_components)
            {
                if (auto castedComponent = std::dynamic_pointer_cast<T>(component))
                {
                    return castedComponent;
                }
            }
            return nullptr;
        }

    private:
        std::vector<SharedPtr<UActorComponent>> m_components;

    public:
        SharedPtr<USceneComponent> RootComponent = nullptr;

    public:
        //raw ptr suggest not managed.
        ULevel* level{ nullptr };
        UWorld* GetWorld() const;

        std::string tag = "defaultTag";
    };

    // factory function for derived classes
    template <DerivedFrom<AActor> T, typename... Args>
    SharedPtr<T> CreateActor(Args&&... args)
    {
        //// abstract class cannot be instantiated
        //static_assert(std::is_base_of<AActor, T>::value, "T must be derived from AActor");

        auto _actor = CreateShared<T>(std::forward<Args>(args)...);

        // register the actor to the components,  post-construction of the actor;
        // 8.4 : it should be delayed until add to world;
        //_actor->RegisterAllComponents(); 

        return _actor;
    }

}