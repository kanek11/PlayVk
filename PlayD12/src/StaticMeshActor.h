#pragma once
#include "PCH.h"
#include "Math/MMath.h"
 

#include "Base.h"
#include "Delegate.h"

#include "Render/Mesh.h"
#include "Render/Texture.h" 

#include "physics/PhysicsScene.h"

struct StaticMeshActorProxy;

struct CameraProxy {
    Float4x4 pvMatrix;

    virtual void Tick(float delta);
};

struct FollowCameraProxy : public CameraProxy {
    Float3 offset = { 0.0f, +5.0f , -10.0f };
    WeakPtr<StaticMeshActorProxy> target;

    virtual void Tick(float delta) override;
};



struct InstanceData
{
    MMath::Float3 offset;
};

struct FInstanceProxy {
    std::vector<InstanceData> instanceData;
    SharedPtr<FD3D12Buffer> instanceBuffer;
};

struct FMaterialProxy {

    SharedPtr<FD3D12Texture> baseMap;
};

//strip out the minimum to render a static mesh:
struct StaticMeshActorProxy {
    std::string debugName = "default static mesh";

    Float3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionIdentity();
    Float3 scale = { 1.0f, 1.0f, 1.0f };

    SharedPtr<UStaticMesh> mesh;

    //material. 
    uint32_t mainPassHeapOffset = 0;
    SharedPtr<FMaterialProxy> material;
    SharedPtr<FD3D12Buffer> mainMVPConstantBuffer;

    uint32_t shadowPassHeapOffset = 0; // for shadow pass
    SharedPtr<FD3D12Buffer> shadowMVPConstantBuffer; // for shadow pass 

    SharedPtr<FInstanceProxy> instanceProxy;

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