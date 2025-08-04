#pragma once

#include "PCH.h"
#include "Gameplay/SceneComponent.h"


namespace Gameplay {
	//all visible shapes ; rigidbody;
	class UPrimitiveComponent : public USceneComponent {
    public: 
        //virtual void Draw() = 0;   
 
        //void SetVisible(bool bVisible);
        //bool IsVisible() const; 
 
        //virtual AABB GetBounds() const;

        void SetSimulatePhysics(bool bSimulate);
        bool IsSimulatingPhysics() const;

        void SetCollisionEnabled(bool bEnabled);
        bool IsCollisionEnabled() const;
        //Set/Get CollisionResponse

        ////for the editor
        //void SetSelected(bool bSelected);
        //bool IsSelected() const;
         
        //void SetRenderLayer(int layer);
        //int  GetRenderLayer() const;

    protected:
        //bool m_bVisible = true;
        bool m_bSimulatePhysics = false;
        bool m_bCollisionEnabled = true;
        //bool m_bSelected = false;
        //int  m_renderLayer = 0;
	};

	class UMeshComponent : public UPrimitiveComponent {
	};

	class UStaticMeshComponent : public UMeshComponent {
         
    //    void SetMesh(SharedPtr<UStaticMesh> mesh);
    //    SharedPtr<UStaticMesh> GetMesh() const;
    //     
    //    void SetMaterial(int slot, SharedPtr<UMaterial> material);
    //    SharedPtr<UMaterial> GetMaterial(int slot = 0) const;
    //     
    //    int  GetMaterialCount() const;

        //extra attributes
        //void SetInstanceData(const std::vector<InstanceData>& instanceData);
        //const std::vector<InstanceData>& GetInstanceData() const;

        // LOD 
        //void SetLOD(int lod);
        //int  GetLOD() const;

        // 
        //void SetCastShadow(bool bCastShadow);
        //bool GetCastShadow() const;

    protected:
        //std::vector<InstanceData> m_instanceData; 
        //int m_currentLOD = 0;
        //bool m_castShadow = true;

    //protected:
    //    SharedPtr<UStaticMesh> m_mesh;
    //    std::vector<SharedPtr<UMaterial>> m_materials;
	};
	 
}