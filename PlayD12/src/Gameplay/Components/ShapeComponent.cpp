#include "PCH.h"
#include "ShapeComponent.h"

#include "Gameplay/World.h"

void Gameplay::USphereComponent::OnRegister()
{
	UShapeComponent::OnRegister();

	std::cout << " sphere shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody;


    auto shape = Sphere{ this->m_radius };

    auto collider = new Collider(shape, rb);
    rb->SetShape(shape);  

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider);

}

void Gameplay::UPlaneComponent::OnRegister()
{
    UShapeComponent::OnRegister();

    std::cout << " plane shape on register\n";

    auto owningWorld = this->GetWorld();
    auto id = this->id;
    auto rb = this->rigidBody; 

	auto shape = Plane{ this->m_size.x(), this->m_size.y()};

    auto collider = new Collider(shape, rb);
    rb->SetShape(shape);

    owningWorld->physicsScene->AddRigidBody(rb, id);
    owningWorld->physicsScene->AddCollider(collider);
}
