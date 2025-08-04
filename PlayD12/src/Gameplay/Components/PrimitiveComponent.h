#pragma once
#include "PCH.h"
#include "Gameplay/SceneComponent.h"

#include "Render/Mesh.h"
#include "Render/Material.h"
#include "Render/StaticMeshProxy.h"

#include "Physics/PhysicsScene.h"


#include <atomic>


namespace Gameplay {

    using FPrimitiveComponentId = uint32_t; 

    //all visible shapes ; rigidbody;
    class UPrimitiveComponent : public USceneComponent {
    public:
        UPrimitiveComponent();

        virtual void OnRegister() override;

        //virtual void Draw() = 0;    

        //void SetVisible(bool bVisible);
        //bool IsVisible() const; 

        //virtual AABB GetBounds() const;

        //void SetSimulatePhysics(bool bSimulate);
        //bool IsSimulatingPhysics() const;

        //void SetCollisionEnabled(bool bEnabled);
        //bool IsCollisionEnabled() const;
        //Set/Get CollisionResponse

        ////for the editor
        //void SetSelected(bool bSelected);
        //bool IsSelected() const;

        //void SetRenderLayer(int layer);
        //int  GetRenderLayer() const;

    protected:
        //bool m_bVisible = true;
        //bool m_bSimulatePhysics = false;
        //bool m_bCollisionEnabled = true;
        //bool m_bSelected = false;
        //int  m_renderLayer = 0;

    public:
        RigidBody* rigidBody = new RigidBody(); 

        static std::atomic<uint32_t> GComponentIdGenerator;
        
    public:
        FPrimitiveComponentId id;
    };

}