#pragma once
#include "PCH.h"
#include "Gameplay/SceneComponent.h"

#include "Render/Mesh.h"
#include "Render/Material.h"
#include "Render/StaticMeshProxy.h"

#include <atomic>

#include "Physics/PhysicsScene.h"
#include "Delegate.h"

namespace Gameplay { 
    using OverlapEvent = FDelegate<void(AActor*)>;


    using FPrimitiveComponentId = uint32_t;

    //all visible shapes ; rigidbody;
    class UPrimitiveComponent : public USceneComponent {
    public:
        UPrimitiveComponent();

        virtual void OnRegister() override;
        virtual void EndPlay() override;

        //virtual void Draw() = 0;    

        //because not only mesh is visible obj.
        void SetVisible(bool visible) { this->bVisible = visible; }
        bool IsVisible() const { this->bVisible; }

        //virtual AABB GetBounds() const;

        void SetSimulatePhysics(bool bSimulate) { bSimulatePhysics = bSimulate; }
        bool IsSimulatingPhysics() const { return bSimulatePhysics;  }
        //Set/Get CollisionResponse

        ////for the editor
        //void SetSelected(bool bSelected);
        //bool IsSelected() const;

        //void SetRenderLayer(int layer);
        //int  GetRenderLayer() const;  

    protected:
        bool bVisible{ true };
        bool bSimulatePhysics{ true };
        //bool m_bCollisionEnabled = true;
        //bool m_bSelected = false;
        //int  m_renderLayer = 0;

    public:
        SharedPtr<RigidBody> rigidBody = CreateShared<RigidBody>();

        static std::atomic<uint32_t> GComponentIdGenerator;
    public:
        FPrimitiveComponentId id{};

        OverlapEvent onOverlap;

        FDelegate<void()> onPrePhysicsEvents;
    };

}