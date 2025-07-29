#pragma once
#include "PCH.h"
#include "Math/MMath.h" 

#include "Base.h"
#include "Delegate.h"

#include "Render/Mesh.h"
#include "Render/Texture.h" 
#include "Render/Material.h" 

#include "physics/PhysicsScene.h"

struct StaticMeshActorProxy;

struct FCameraProxy {
    Float4x4 pvMatrix;
	Float4x4 invProjMatrix;
	Float4x4 invViewMatrix;
    Float3 position;

    virtual void Tick(float delta);
};

struct FollowCameraProxy : public FCameraProxy {
    Float3 offset = { 0.0f, +2.0f , -5.0f };
    WeakPtr<StaticMeshActorProxy> target;

    virtual void Tick(float delta) override;
};



struct InstanceData
{
    MMath::Float3 offset;
};

template <>
struct VertexLayoutTraits<InstanceData> {
    static constexpr bool is_specialized = true;
    static constexpr std::array<VertexAttribute, 1> attributes = {
        VertexAttribute{ "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(InstanceData, offset) }
    };
};



//strip out the minimum to render a static mesh:
struct StaticMeshActorProxy {
    std::string debugName = "default static mesh";

    Float3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionIdentity();
    Float3 scale = { 1.0f, 1.0f, 1.0f };

    Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

    SharedPtr<UStaticMesh> mesh;

    SharedPtr<FMaterialProxy> material;


    //uint32_t shadowPassHeapOffset = 0; // for shadow pass
    //SharedPtr<FD3D12Buffer> shadowMVPConstantBuffer; // for shadow pass 

    std::vector<InstanceData> instanceData;

    ////new: for physics:
    RigidBody* rigidBody{ nullptr };
    Collider* collider{ nullptr };

    void SetWorldPosition(const Float3& newPosition) {
        position = newPosition;
    }

    void SetWorldRotation(const DirectX::XMVECTOR& newRotation) {
        rotation = newRotation;
    }


    FDelegate<void(float)> onUpdate;
};




SharedPtr<StaticMeshActorProxy> CreateStaticMeshActor(SharedPtr<UStaticMesh> mesh, Float3 position = { 0.0f, 0.0f, 0.0f }, Float3 scale = { 1.0f, 1.0f, 1.0f });

SharedPtr<StaticMeshActorProxy> CreateSphereActor(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });
SharedPtr<StaticMeshActorProxy> CreateBoxActor(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });

SharedPtr<StaticMeshActorProxy> CreatePlaneActor(Float3 position, Float3 scale, uint32_t subdivisionX, uint32_t subdivisionZ);