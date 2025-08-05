#pragma once
#include "PCH.h"
#include "Math/MMath.h" 

#include "Base.h"
#include "Delegate.h"

#include "Render/Mesh.h"
#include "Render/Texture.h" 
#include "Render/Material.h" 

#include "physics/PhysicsScene.h"

#include "Gameplay/Actor.h"
//#include "Gameplay/Components/SceneComponent.h"
#include "Gameplay/Components/MeshComponent.h"
#include "Gameplay/Components/ShapeComponent.h"


struct StaticMeshActorProxy;

struct FCameraProxy {
    Float4x4 pvMatrix;
    Float4x4 invProjMatrix;
    Float4x4 invViewMatrix;
    Float3 position;

    virtual void Tick(float delta);
};

struct FollowCameraProxy : public FCameraProxy {
    Float3 offset = { 0.0f, +5.0f , -5.0f };
    WeakPtr<StaticMeshActorProxy> target;

    virtual void Tick(float delta) override;
};


//strip out the minimum to render a static mesh:
struct StaticMeshActorProxy {
    std::string debugName = "default static mesh";

    //transform
    Float3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionIdentity();
    Float3 scale = { 1.0f, 1.0f, 1.0f };

    Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

    //static mesh
    SharedPtr<UStaticMesh> mesh;

    SharedPtr<UMaterial> material;

    std::vector<InstanceData> instanceData;

    //new: for physics:
    RigidBody* rigidBody{ nullptr };
    Collider* collider{ nullptr };

    void SetWorldPosition(const Float3& newPosition) {
        position = newPosition;
    }

    void SetWorldRotation(const DirectX::XMVECTOR& newRotation) {
        rotation = newRotation;
    }

    //custom behavior;
    FDelegate<void(float)> onUpdate;
};

SharedPtr<StaticMeshActorProxy> CreateStaticMeshActor(SharedPtr<UStaticMesh> mesh, Float3 position = { 0.0f, 0.0f, 0.0f }, Float3 scale = { 1.0f, 1.0f, 1.0f });

SharedPtr<StaticMeshActorProxy> CreateSphereActor(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });
SharedPtr<StaticMeshActorProxy> CreateBoxActor(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });

SharedPtr<StaticMeshActorProxy> CreatePlaneActor(Float3 position, Float3 scale, uint32_t subdivisionX, uint32_t subdivisionZ);




using namespace Gameplay;

class ACameraActor : public AActor
{
public:

};

//todo: object initializer or sth;
class AStaticMeshActor : public AActor
{
public:
    AStaticMeshActor();

    virtual void OnTick(float delta);

public:
    SharedPtr<UStaticMeshComponent> staticMeshComponent;
    SharedPtr<UShapeComponent> shapeComponent;  
};


namespace Mesh {

    SharedPtr<AStaticMeshActor> CreateStaticMeshActor(SharedPtr<UStaticMesh> mesh, Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });

    SharedPtr<AStaticMeshActor> CreateSphere(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });
    SharedPtr<AStaticMeshActor> CreateBox(Float3 position, Float3 scale = { 1.0f, 1.0f, 1.0f });
    SharedPtr<AStaticMeshActor> CreatePlane(Float3 position, Float3 scale, uint32_t subdivisionX, uint32_t subdivisionZ);

}


constexpr int instanceCount = 1;
constexpr float spacing = 5.0f; // Adjust to control sparsity
constexpr float viewRadius = 7.0f; // Distance from the origin  

inline std::vector<InstanceData> DebugGenerateInstanceData()
{
    std::vector<InstanceData> instanceData;

    // Estimate cube grid dimensions (cubical or close)
    int gridSize = static_cast<int>(std::ceil(std::cbrt(instanceCount)));
    const float halfGrid = (gridSize - 1) * spacing * 0.5f;

    for (int i = 0; i < instanceCount; ++i)
    {
        int x = i % gridSize;
        int y = (i / gridSize) % gridSize;
        int z = i / (gridSize * gridSize);

        InstanceData _data;

        _data.offset = {
            x * spacing - halfGrid,
            y * spacing - halfGrid,
            z * spacing - halfGrid
        };

        instanceData.push_back(_data);
    }

    return instanceData;
}