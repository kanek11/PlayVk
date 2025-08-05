#include "PCH.h"
#include "ShapeComponent.h"

#include "Gameplay/World.h"


//----------------
void Gameplay::USphereComponent::OnRegister()
{
	UShapeComponent::OnRegister();

	std::cout << " sphere shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody; 

    auto shape = Sphere{ this->radius };

    auto collider = new Collider(shape, rb);
    rb->SetShape(shape);  

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider);

}

//----------------
 

void Gameplay::UPlaneComponent::OnRegister()
{
    UShapeComponent::OnRegister();

    std::cout << " plane shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody; 

	auto shape = Plane{ this->size.x(), this->size.y()};

    auto collider = new Collider(shape, rb);
    rb->SetShape(shape);

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider);
}


//------------
 

void Gameplay::UBoxComponent::OnRegister()
{
	UShapeComponent::OnRegister();

	std::cout << " box shape on register\n";
	auto owningWorld = this->GetWorld();
	auto id = this->id;
	auto rb = this->rigidBody;

	auto shape = Box(Float3{ this->extents.x()* 0.5f, this->extents.y() * 0.5f, this->extents.z() * 0.5f });

	auto collider = new Collider(shape, rb);
	rb->SetShape(shape);
	owningWorld->physicsScene->AddRigidBody(rb, id);
	owningWorld->physicsScene->AddCollider(collider);
}
