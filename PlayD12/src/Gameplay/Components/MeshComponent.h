#pragma once

#include "PCH.h"
#include "Gameplay/SceneComponent.h"

#include "Render/Mesh.h"
#include "Render/Material.h"
#include "Render/StaticMeshProxy.h"

#include "PrimitiveComponent.h"

namespace Gameplay {

    class UMeshComponent : public UPrimitiveComponent {
    public:
    };

    class UStaticMeshComponent : public UMeshComponent {
    public:
        virtual void TickComponent(float delta);

        virtual void OnRegister() override;
        FStaticMeshProxy CreateSceneProxy();

        void SetMesh(SharedPtr<UStaticMesh> mesh)
        {
            m_mesh = mesh;
        }
        SharedPtr<UStaticMesh> GetMesh() const {
            return m_mesh;
        }

        //todo: implement multi-material if needed;
        void SetMaterial(SharedPtr<UMaterial> material, int slot = 0) {
            m_material = material;
        }
        SharedPtr<UMaterial> GetMaterial(int slot = 0) const
        {
            return m_material;
        }
        //int  GetMaterialCount() const;

        //extra attributes
        //void SetInstanceData(const std::vector<InstanceData>& instanceData);
        //const std::vector<InstanceData>& GetInstanceData() const;

        // LOD 
        //void SetLOD(int lod);
        //int  GetLOD() const;
         
        //void SetCastShadow(bool bCastShadow);
        //bool GetCastShadow() const;

    protected:
        //std::vector<InstanceData> m_instanceData; 
        //int m_currentLOD = 0;
        //bool m_castShadow = true;

    protected:
        SharedPtr<UStaticMesh> m_mesh{ nullptr };
        SharedPtr<UMaterial> m_material = CreateShared<UMaterial>();
        //std::vector<SharedPtr<UMaterial>> m_materials;

    protected:
        //cache 
        FStaticMeshProxy m_sceneProxy;
    };

}