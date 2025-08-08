#include "PCH.h"
#include "ShapeComponent.h"

#include "Gameplay/World.h"


//----------------
namespace Gameplay {
    UShapeComponent::UShapeComponent():UPrimitiveComponent()
    { 
        collider = CreateShared<Collider>(rigidBody);
    }

    void UShapeComponent::OnRegister()
{
    UPrimitiveComponent::OnRegister(); 

    std::cout << " sphere shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody;  
    rb->SetShape(shape);

    collider->SetShape(shape);
    collider->SetIsTrigger(this->bIsTrigger);

    owningWorld->physicsScene->SetRigidBody(rb, id);
    owningWorld->physicsScene->SetCollider(collider.get(), id);
}

void UShapeComponent::SetShape(const ShapeType& shape)
{
    this->shape = shape; 

    //for now, this means the setshape is called before registery which is not wrong;
    //if invoke after registry, then it set back to physics scene auto;
    auto owningWorld = this->GetWorld();
    if (!owningWorld) return;
    auto id = this->id;
    auto rb = this->rigidBody;
    rb->SetShape(shape);

    collider->SetShape(shape);

    owningWorld->physicsScene->SetRigidBody(rb, id);
    owningWorld->physicsScene->SetCollider(collider.get(), id);

}



/*
void Gameplay::USphereComponent::OnRegister()
{
    UShapeComponent::OnRegister();

    std::cout << " sphere shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody;

    shape = Sphere{ this->radius }; 

    collider = CreateShared<Collider>(shape, rb);
    rb->SetShape(shape); 

    collider->SetIsTrigger(this->bIsTrigger);

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider.get(), id);

}

//----------------


void Gameplay::UPlaneComponent::OnRegister()
{
    UShapeComponent::OnRegister();

    std::cout << " plane shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody;

    auto shape = Plane{ this->size.x(), this->size.y() };

    auto collider = new Collider(shape, rb);
    rb->SetShape(shape);

    collider->SetIsTrigger(this->bIsTrigger);

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider, id);
}


//------------


void Gameplay::UBoxComponent::OnRegister()
{
    //UShapeComponent::OnRegister();

    //std::cout << " box shape on register\n";
    //auto owningWorld = this->GetWorld();
    //auto id = this->id;
    //auto rb = this->rigidBody;

    //auto shape = Box(Float3{ this->extents.x() * 0.5f, this->extents.y() * 0.5f, this->extents.z() * 0.5f });
    //rb->SetShape(shape);

    //auto collider = new Collider(shape, rb);
    //collider->SetIsTrigger(this->bIsTrigger);

    //
    //owningWorld->physicsScene->AddRigidBody(rb, id);
    //owningWorld->physicsScene->AddCollider(collider, id);
}
*/

}