#include "PCH.h"
#include "StaticMeshActor.h"
#include "Application.h" 

#include "Physics/PhysicsScene.h"


constexpr int instanceCount = 1;
constexpr float spacing = 5.0f; // Adjust to control sparsity
constexpr float viewRadius = 7.0f; // Distance from the origin  

std::vector<InstanceData> DebugGenerateInstanceData()
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


SharedPtr<StaticMeshActorProxy> CreateStaticMeshActor(SharedPtr<UStaticMesh> mesh,
    Float3 position,
    Float3 scale
)
{
    auto ctx = Render::rendererContext;
    if (ctx) {
        mesh->CreateGPUResource(ctx->device);
    }


    //------------------------------  

    //auto shadowPassOffset = ctx->shadowShaderPerm->RequestAllocationOnHeap();
    //std::cout << "Heap start offset for shadow pass: " << shadowPassOffset << std::endl;

    // 

    ////--------------------------
    //auto shadowConstBuffer = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
    //    sizeof(MVPConstantBuffer),
    //    DXGI_FORMAT_UNKNOWN, // Not used for constant buffers
    //    256, // Alignment
    //    EBufferUsage::Upload | EBufferUsage::Constant
    //    });

    //MVPConstantBuffer shadowConstantBufferData{};
    //shadowConstBuffer->UploadData(&shadowConstantBufferData, sizeof(shadowConstantBufferData));


    //------------------------------
    auto instanceData = DebugGenerateInstanceData();
    UINT instanceBufferSize = static_cast<UINT>(sizeof(InstanceData) * instanceData.size());

    //------------------------------
    auto meshProxy = CreateShared<StaticMeshActorProxy>();
    meshProxy->position = position;
    meshProxy->scale = scale;
    meshProxy->mesh = mesh;
    //meshProxy->mainPassHeapOffset = mainPassOffset;
    meshProxy->material = CreateShared<FMaterialProxy>();
    //meshProxy->objectCB = objectCBRes;
    //meshProxy->shadowPassHeapOffset = shadowPassOffset;
    //meshProxy->shadowMVPConstantBuffer = shadowConstBuffer;
    //meshProxy->instanceProxy = CreateShared<FInstanceProxy>(instanceData, instanceBuffer); 

    meshProxy->instanceData = std::move(instanceData);

    return meshProxy;
}


SharedPtr<StaticMeshActorProxy> CreateSphereActor(Float3 position, Float3 scale)
{
  
    auto sphereMesh = CreateShared<SphereMesh>();
    auto sphereProxy = CreateStaticMeshActor(sphereMesh, position, scale);

    auto sphereShape = Sphere{ sphereProxy->scale.x() };
    //auto sphereRigidBody = new RigidBody(sphereProxy.get(), sphereShape); 

    //auto sphereCollider = new Collider(sphereProxy.get(), sphereShape, sphereRigidBody); 
    auto sphereRigidBody = new RigidBody();

    auto sphereCollider = new Collider(sphereShape, sphereRigidBody);
	sphereRigidBody->SetShape(sphereShape); 

    sphereProxy->collider = sphereCollider;
    sphereProxy->rigidBody = sphereRigidBody;


    return sphereProxy;
}

SharedPtr<StaticMeshActorProxy> CreateBoxActor(Float3 position, Float3 scale)
{ 
    auto boxMesh = CreateShared<CubeMesh>();
    auto boxProxy = CreateStaticMeshActor(boxMesh, position, scale);

    auto boxShape = Box{ boxProxy->scale };
    //auto boxRigidBody = new RigidBody(boxProxy.get(), boxShape);

    //auto boxCollider = new Collider(boxProxy.get(), boxShape, boxRigidBody);

    auto boxRigidBody = new RigidBody();

    auto boxCollider = new Collider(boxShape, boxRigidBody);
	boxRigidBody->SetShape(boxShape); 

    boxProxy->collider = boxCollider;
    boxProxy->rigidBody = boxRigidBody;


    return boxProxy;
}

SharedPtr<StaticMeshActorProxy> CreatePlaneActor(Float3 position, Float3 scale, uint32_t subdivisionX, uint32_t subdivisionZ)
{
    auto planeMesh = CreateShared<PlaneMesh>(subdivisionX, subdivisionZ);
    auto planeProxy = CreateStaticMeshActor(planeMesh, position, scale);

    auto planeShape = Plane{ static_cast<float>(subdivisionX) , static_cast<float>(subdivisionZ) };
    
    //auto planeRigidBody = new RigidBody(planeProxy.get(), planeShape);

    //auto planeCollider = new Collider(planeProxy.get(), planeShape, planeRigidBody);

    auto planeRigidBody = new RigidBody(); 
    auto planeCollider = new Collider(planeShape, planeRigidBody);
    planeRigidBody->SetShape(planeShape);

    planeProxy->collider = planeCollider;
    planeProxy->rigidBody = planeRigidBody;

    return planeProxy;
}

void FCameraProxy::Tick(float delta)
{
    //eye rotate around the origin
    constexpr float speedDivisor = 50.0f; // Increase this number to slow it down
    float angle = static_cast<float>((GetTickCount64() / static_cast<ULONGLONG>(speedDivisor)) % 360) * MMath::PI / 180.0f;

    float eyePosX = cos(angle) * viewRadius;
    float eyePosY = viewRadius * 0.3f;
    float eyePosZ = sin(angle) * viewRadius;

    //float eyePosX = viewRadius * 0.3;
    //float eyePosY = cos(angle) * viewRadius ;
    //float eyePosZ = sin(angle) * viewRadius;

    //float eyePosX = -viewRadius * 0.3;
    //float eyePosY = viewRadius * 0.2;
    //float eyePosZ = -viewRadius * 0.3;

    //float eyePosX = -1.0f;
    //float eyePosY = 5.0f;
    //float eyePosZ = -10.0;

    //Create view and projection matrices
   //LH = left-handed coordinate system
    //XMMATRIX view = XMMatrixLookAtLH(
    //    XMVectorSet(eyePosX, eyePosY, eyePosZ, 1.0f),  // Eye
    //    XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // At
    //    XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // Up
    //);

    auto view_ = MMath::LookAtLH(
        { eyePosX, eyePosY, eyePosZ }, // Eye
        { 0.0f, 0.0f, 0.0f },          // At
        { 0.0f, 1.0f, 0.0f }           // Up
    );

    float aspectRatio = GameApplication::GetInstance()->getAspectRatio();
    //XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
    auto proj_ = MMath::PerspectiveFovLH(
        MMath::ToRadians(45.0f),
        aspectRatio,
        0.1f,
        1000.0f
    );

    auto invView_ = MMath::InverseLookAtLH
    (
        { eyePosX, eyePosY, eyePosZ }, // Eye
        { 0.0f, 0.0f, 0.0f },          // At
        { 0.0f, 1.0f, 0.0f }           // Up
    );


    auto invProj_ = MMath::InversePerspectiveFovLH(
        MMath::ToRadians(45.0f),
        aspectRatio,
        0.1f,
        1000.0f
    );

    this->pvMatrix = MatrixMultiply(proj_, view_);
    this->invViewMatrix = invView_;
    this->invProjMatrix = invProj_;
    this->position = { eyePosX, eyePosY, eyePosZ };

    //std::cout << "view matrix:\n " << MMath::ToString(view_) << std::endl;
    //std::cout << "inv view matrix:\n " << MMath::ToString(invView_) << std::endl;
}

void FollowCameraProxy::Tick(float delta)
{
    if (target.expired()) {
        std::cerr << "no valid actor" << std::endl;
        return;
    }

    Float3 targetPos = target.lock()->position;
    Float3 eyePos = targetPos + offset;

    auto view_ = MMath::LookAtLH(
        eyePos, // Eye
        targetPos,          // At
        { 0.0f, 1.0f, 0.0f }           // Up
    );

    auto invView_ = MMath::InverseLookAtLH
    (
        eyePos, // Eye
        targetPos,          // At
        { 0.0f, 1.0f, 0.0f }           // Up
    );


    float aspectRatio = GameApplication::GetInstance()->getAspectRatio();

    auto proj_ = MMath::PerspectiveFovLH(
        MMath::ToRadians(45.0f),
        aspectRatio,
        0.1f,
        1000.0f
    );

    auto invProj_ = MMath::InversePerspectiveFovLH(
        MMath::ToRadians(45.0f),
        aspectRatio,
        0.1f,
        1000.0f
    );

    this->pvMatrix = MatrixMultiply(proj_, view_);
    this->invViewMatrix = invView_;
    this->invProjMatrix = invProj_;
    this->position = eyePos;

    //std::cout << "view matrix:\n " << MMath::ToString(view_) << std::endl;
    //std::cout << "inv view matrix:\n " << MMath::ToString(invView_) << std::endl;  
 //   //std::cout << "inv view matrix ref:\n " << MMath::ToString(MMath::Inverse4x4(view_)) << std::endl;

    //std::cout << "projection matrix:\n " << MMath::ToString(proj_) << std::endl;
    //std::cout << "inv proj matrix:\n " << MMath::ToString(invProj_) << std::endl;
    //std::cout << "inv proj ref:\n " << MMath::ToString(MMath::Inverse4x4(proj_)) << std::endl;
}
