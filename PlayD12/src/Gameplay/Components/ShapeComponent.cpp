#include "PCH.h"
#include "ShapeComponent.h"

#include "Gameplay/World.h"


//----------------
namespace Gameplay { 


    UShapeComponent::UShapeComponent() :UPrimitiveComponent()
    {
        assert(rigidBody != nullptr);
        collider = CreateShared<Collider>(rigidBody.get());
    }

    void UShapeComponent::OnRegister()
    {
        UPrimitiveComponent::OnRegister();

        std::cout << " shape on register\n";

        this->UploadStateToPhysics();
    }


    void UShapeComponent::SetShape(const ShapeType& shape)
    {
        this->shape = shape;

        this->UploadStateToPhysics();
    }



    void UShapeComponent::EndPlay()
    {
        UPrimitiveComponent::EndPlay();

        auto owningWorld = this->GetWorld();
        auto id = this->id;

        owningWorld->physicsScene->RemoveRigidBody(id);
        owningWorld->physicsScene->RemoveCollider(id);
    }


    void UShapeComponent::SetColliderShape(const ShapeType& shape)
    {
        auto owningWorld = this->GetWorld();
        if (!owningWorld) {
            std::cout << "owning world not set; could be unexpected?" << '\n';
            return;
        }
        auto id = this->id;

        owningWorld->physicsScene->SetColliderShape(id, shape);
    }

    void UShapeComponent::UploadStateToPhysics()
    {

        auto owningWorld = this->GetWorld();
        if (!owningWorld) {
            //std::cout << "owning world not set; could be unexpected?" << '\n';
            return;
        }

        auto id = this->id;
        auto rb = this->rigidBody.get();

        collider->SetIsTrigger(this->bIsTrigger);

        owningWorld->physicsScene->AddRigidBody(rb, id);
        owningWorld->physicsScene->AddCollider(collider.get(), id);

        owningWorld->physicsScene->SetShape(id, shape);

        owningWorld->physicsScene->SetPosition(id, this->GetWorldPosition());
        owningWorld->physicsScene->SetRotation(id, this->GetWorldRotation());

    }


}