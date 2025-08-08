#pragma once 
#include "PCH.h"

#include "Gameplay/SceneComponent.h"
#include "PrimitiveComponent.h"

#include "Physics/PhysicsScene.h"

namespace Gameplay {


    class UShapeComponent : public UPrimitiveComponent {
    public:
        UShapeComponent();
        virtual void OnRegister() override;
        //for debug
        //virtual void SetShapeColor(const Float4& color);
        //virtual Float4 GetShapeColor() const;

        //void SetDrawDebug(bool bDraw);

        //physical volume for advanced simulation;
        //virtual float GetVolume() const = 0; 

        //// 
        //void SetDensity(float density);
        //float GetDensity() const;
        void SetShape(const ShapeType& shape);

        void SetIsTrigger(bool isTrigger) { this->bIsTrigger = isTrigger; } 

         
        void SetCollisionEnabled(bool bEnabled) { collider->bEnabled = bEnabled; }
        bool IsCollisionEnabled() const { return collider->bEnabled; }

    protected:
        Float4 m_shapeColor = { 1,1,1,0.5f };
        bool   m_drawDebug = true;
        // float  m_density = 1.0f; 

        bool bIsTrigger{ false };

    public:
        ShapeType shape;  
        SharedPtr<Collider> collider;
    };


    /*
    class UBoxComponent : public UShapeComponent {
        void SetBoxExtent(const Float3& extent) { extents = extent; }
        Float3 GetBoxExtent() const { return extents; };

        Float3 extents = { 1.0f,1.0f,1.0f };
        virtual void OnRegister() override;
    };

    class USphereComponent : public UShapeComponent {
        void SetSphereRadius(float r) { radius = r; }
        float GetSphereRadius() const { return radius; };

        virtual void OnRegister() override;

        float radius = 1.0f;
    };

    class UPlaneComponent : public UShapeComponent {
    public:

        void SetPlaneSize(const Float2& size) {
            this->size = size;
        }
        void SetPlaneSize(float width, float height) {
            this->size = { width, height };
        }
        Float2 GetPlaneSize() const { return size; }

        virtual void OnRegister() override;

        Float2 size = { 1.0f, 1.0f };
    };
    */

}