#pragma once 
#include "PCH.h"

#include "Gameplay/SceneComponent.h"
#include "PrimitiveComponent.h"

namespace Gameplay {


    class UShapeComponent : public UPrimitiveComponent {

        //for debug
        //virtual void SetShapeColor(const Float4& color);
        //virtual Float4 GetShapeColor() const;

        //void SetDrawDebug(bool bDraw);

        //physical volume for advanced simulation;
        //virtual float GetVolume() const = 0; 

        //// 
        //void SetDensity(float density);
        //float GetDensity() const;

    protected:
        Float4 m_shapeColor = { 1,1,1,0.5f };
        bool   m_drawDebug = true;
        // float  m_density = 1.0f; 
    };

    class UBoxComponent : public UShapeComponent {
		void SetBoxExtent(const Float3& extent) { extents = extent; }
		Float3 GetBoxExtent() const { return extents; };

        Float3 extents = { 1.0f,1.0f,1.0f };
        virtual void OnRegister() override;
    };

    class USphereComponent : public UShapeComponent {
        void SetSphereRadius(float r) { radius = r; }
        float GetSphereRadius() const {  return radius; };

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


}