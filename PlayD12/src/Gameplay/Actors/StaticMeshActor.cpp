#include "PCH.h"
#include "Application.h" 

#include "Physics/PhysicsScene.h"

#include "StaticMeshActor.h"
//todo: 
/*
register the mesh as assets
*/


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

    Mesh::SetSphere(actor.get(), radius); 
     
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
