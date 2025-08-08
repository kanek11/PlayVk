#include "PCH.h"
#include "Application.h" 

#include "Physics/PhysicsScene.h"

#include "StaticMeshActor.h"
//todo: 
/*
register the mesh as assets
*/

 
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
        std::cerr << "follow camera: empty target" << std::endl;
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
    //std::cout << "inv view matrix ref:\n " << MMath::ToString(MMath::Inverse4x4(view_)) << std::endl;

    //std::cout << "projection matrix:\n " << MMath::ToString(proj_) << std::endl;
    //std::cout << "inv proj matrix:\n " << MMath::ToString(invProj_) << std::endl;
    //std::cout << "inv proj ref:\n " << MMath::ToString(MMath::Inverse4x4(proj_)) << std::endl;
}

AStaticMeshActor::AStaticMeshActor()
{ 
    this->staticMeshComponent = this->CreateComponentAsSubObject<UStaticMeshComponent>();
    this->shapeComponent      = this->CreateComponentAsSubObject<UShapeComponent>(); 
    this->RootComponent = this->shapeComponent; 
    this->staticMeshComponent->AttachTo(this->RootComponent.get());  
}

void AStaticMeshActor::OnTick(float delta)
{
    AActor::OnTick(delta);

    //std::cout << "tick static mesh\n";
}

//SharedPtr<AStaticMeshActor> Mesh::CreateStaticMeshActor(SharedPtr<UStaticMesh> mesh, Float3 position, Float3 scale)
//{
//    auto& actor = CreateActor<AStaticMeshActor>();
//    actor->staticMeshComponent->SetMesh(mesh);
//    return actor;
//}

 
void Mesh::SetSphere(AActor* actor, float radius)
{
    auto mesh = CreateShared<SphereMesh>();
    auto shape = Sphere{ radius }; 

    if (auto meshComp = actor->GetComponent<UStaticMeshComponent>()) {

        meshComp->SetRelativeScale(Float3(radius, radius, radius));
        meshComp->SetMesh(mesh);
    }

    if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
        shapeComp->SetShape(shape);
    } 
}

void Mesh::SetPlane(AActor* actor, uint32_t subdivisionX, uint32_t subdivisionZ)
{
    auto mesh = CreateShared<PlaneMesh>(subdivisionX, subdivisionZ); ;
    auto shape = Plane{ (float)subdivisionX, (float)subdivisionZ };

    if (auto meshComp = actor->GetComponent<UStaticMeshComponent>()) {
        meshComp->SetMesh(mesh);
    }

    if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
        shapeComp->SetShape(shape);
    }
}

void Mesh::SetBox(AActor* actor, Float3 extents)
{
    auto mesh = CreateShared<CubeMesh>(); ;
    auto scale = Float3{ extents.x() * 0.5f, extents.y() * 0.5f, extents.z() * 0.5f };
    auto shape = Box(scale);

    if (auto meshComp = actor->GetComponent<UStaticMeshComponent>()) {
        meshComp->SetRelativeScale(scale);
        meshComp->SetMesh(mesh);
    }

    if (auto shapeComp = actor->GetComponent<UShapeComponent>()) {
        shapeComp->SetShape(shape);
    }
}

SharedPtr<AStaticMeshActor> Mesh::CreateSphereActor(float radius, Float3 position, Float3 scale)
{
    auto actor = CreateActor<AStaticMeshActor>(); 

    SetSphere(actor.get(), radius); 
     
    actor->RootComponent->SetRelativePosition(position);
    actor->RootComponent->SetRelativeScale(scale); 

    //update the world transform manually:
    actor->RootComponent->UpdateWorldTransform();

    return actor;
}

SharedPtr<AStaticMeshActor> Mesh::CreatePlaneActor(uint32_t subdivisionX, uint32_t subdivisionZ , Float3 position, Float3 scale)
{
    auto actor = CreateActor<AStaticMeshActor>(); 

    SetPlane(actor.get(), subdivisionX, subdivisionZ); 

    actor->RootComponent->SetRelativePosition(position);
    actor->RootComponent->SetRelativeScale(scale); 

    //update the world transform manually:
    actor->RootComponent->UpdateWorldTransform();

    return actor;
}

SharedPtr<AStaticMeshActor> Mesh::CreateBoxActor(Float3 extents, Float3 position, Float3 scale)
{
    auto actor = CreateActor<AStaticMeshActor>(); 

    SetBox(actor.get(), extents);  

    actor->RootComponent->SetRelativePosition(position);
    actor->RootComponent->SetRelativeScale(scale);

    //update the world transform manually:
    actor->RootComponent->UpdateWorldTransform();


    return actor;
}
