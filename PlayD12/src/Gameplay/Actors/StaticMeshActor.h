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

    void SetSphere(AActor* actor, float radius);
    void SetPlane(AActor* actor, uint32_t subdivisionX, uint32_t subdivisionZ);
    void SetBox(AActor* actor, Float3 extents);


    SharedPtr<AStaticMeshActor> CreateSphereActor(float radius, Float3 position = { 0.0f, 0.0f, 0.0f }, Float3 scale = { 1.0f, 1.0f, 1.0f });
    SharedPtr<AStaticMeshActor> CreatePlaneActor(uint32_t subdivX, uint32_t subdivZ, Float3 position = { 0.0f, 0.0f, 0.0f }, Float3 scale = { 1.0f, 1.0f, 1.0f });

    SharedPtr<AStaticMeshActor> CreateBoxActor(Float3 extents, Float3 position = { 0.0f, 0.0f, 0.0f }, Float3 scale = { 1.0f, 1.0f, 1.0f });

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