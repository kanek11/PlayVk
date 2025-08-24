#include "PCH.h"
#include "DemoLevels.h"

#include "Application.h"

#include "Gameplay/World.h"
#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"

#include "Item.h"  

#include "Physics/PhysicsScene.h"

/*
==stage
    ////
    //{
    //    float height = 1.0f;
    //    Float3 refPos = end1 + Float3(0.0f, -height, step2 / 5);
    //    Float3 refShape = { roadWidth, height, 3.0f };

    //    auto actor = Mesh::CreateBoxActor(refShape);

    //    RotateAroundXAxis(actor, refPos, refShape.z(), -20.0f);
    //    //ApplySnow(actor);

    //    this->AddActor(actor);
    //}


    {

        Float3 dim = { 3.0f, 3.0f, 0.2f };
        Float3 pos = Float3(0.0f, dim.y() / 2 + 0.2f, 5.0f);

        {
            auto actor = CreateActor<ARotateBox>();
            Mesh::SetBox(actor.get(), dim);
            actor->SetRotationSpeed(45.0f);

            actor->RootComponent->SetRelativePosition(pos);
            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

            this->AddActor(actor);
        }



        {
            auto actor = CreateActor<ARotateBox>();
            Mesh::SetBox(actor.get(), dim);
            actor->SetRotationSpeed(45.0f);

            actor->RootComponent->SetRelativePosition(pos);
            auto rotation = XMQuaternionRotationRollPitchYaw(0.0f, XMConvertToRadians(90.0f), 0.0f);
            actor->RootComponent->SetRelativeRotation(rotation);

            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

            this->AddActor(actor);
        }

    }

    
*/


void MarkAsFloor(SharedPtr<AStaticMeshActor> actor) { 
    actor->tag = "floor";
    actor->shapeComponent->collider->bNeedsEvent = true;
}

void ApplySnow(SharedPtr<AStaticMeshActor> actor) {

    actor->staticMeshComponent->SetMaterial(Materials::GetSnowSurface());

    actor->shapeComponent->rigidBody->SetPhysicalMaterial(PhysicalMaterial{ 0.0f, 0.0f });
}



using namespace DirectX;

void GlobalLevel::OnLoad()
{
    ULevel::OnLoad();
    //new: 
    owningWorld->CreateGameState<AGameState>(this);
    //auto gameState = owningWorld->GetGameState<AGameState>(); 

}

void GlobalLevel::OnUnload()
{
}

void GlobalLevel::OnTick(float delta)
{
    ULevel::OnTick(delta);
}



void GamePlayLevel::OnLoad()
{
    ULevel::OnLoad();

    assert(owningWorld->physicsScene != nullptr);

    //this->Load1();
    this->LoadPlayer();
    this->LoadMap0();
    this->LoadActors();
}


void GamePlayLevel::LoadActors()
{
    //plane:

    // 
    //auto sphereActor = Mesh::CreateSphereActor( 1.0f, Float3{ 2.0f, 2.0f, 0.0f });

    //auto& sphereRB = sphereActor->shapeComponent->rigidBody;
    //sphereRB->simulatePhysics = false;
    //sphereRB->simulateRotation = false; 
    //sphereActor->shapeComponent->SetIsTrigger(true); 
    //sphereActor->staticMeshComponent->SetMaterial(Materials::GetRustyIron()); 


    //auto itemActor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, { 0.0f, 2.0f, 4.0f }); 
    //auto itemActor = CreateShared<AItem>();
    //itemActor->RootComponent->SetRelativePosition({ 0.0f, 1.0f, 4.0f });
    //itemActor->RootComponent->UpdateWorldTransform();






    //  {

          //Float3 pos{ 0.0f, 2.0f, 0.0f };
    //      auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, pos);
    //       
    //      auto rotation = XMQuaternionRotationRollPitchYaw(0.0f,0.0f, XMConvertToRadians(30.0f));
    //      actor->RootComponent->SetRelativeRotation(rotation);  
    //      actor->RootComponent->UpdateWorldTransform();

    //      auto& rb = actor->shapeComponent->rigidBody; 
    //      rb->simulatePhysics = true;
    //      rb->simulateRotation = false;  
    //      this->AddActor(actor);
    //  }

    //  {
    //      Float3 pos{ 0.0f, 4.0f, 0.0f };
    //      auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, pos);

    //      auto rotation = XMQuaternionRotationRollPitchYaw(0.0f, XMConvertToRadians(90.0f), XMConvertToRadians(45.0f));
    //      actor->RootComponent->SetRelativeRotation(rotation);
    //      actor->RootComponent->UpdateWorldTransform();

    //      auto& rb = actor->shapeComponent->rigidBody;
    //      rb->simulatePhysics = true;
    //      rb->simulateRotation = false;   
    //      this->AddActor(actor);
    //  } 

}
 


Float3 RotateAroundXAxis(SharedPtr<AActor> actor, Float3 pivot, float radius, float angle) {

    auto rotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(angle), 0.0f, 0.0f);
    actor->RootComponent->SetRelativeRotation(rotation);

    auto currPos = actor->RootComponent->GetRelativePosition();
    auto offsetY = -radius * std::sin(MMath::ToRadians(angle)) / 2;
    auto offsetZ = radius * std::cos(MMath::ToRadians(angle)) / 2;
    actor->RootComponent->SetRelativePosition(currPos + Float3{ 0.0f, pivot.y() + offsetY , pivot.z() + offsetZ });

    actor->RootComponent->UpdateWorldTransform();

    return { 0.0f, pivot.y() + 2 * offsetY , pivot.z() + 2 * offsetZ };
};

void GamePlayLevel::LoadMap0()
{
    
    float step0 = 20.0f;
    Float3 end0{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)roadWidth, (uint32_t)step0);
        ApplySnow(actor);  
        MarkAsFloor(actor);

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end0 = RotateAroundXAxis(actor, { 0.f,0.f,0.f }, step0, 0.0f);

        this->AddActor(actor);
    }


    {
        auto& grid = MMath::GenerateGrid3D({ 1,1,3 }, { 1,1,5.0f });
        Float3 offset = { 0.0 ,1.0f, 0.0f };
        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto iceItem = CreateShared<AIceItem>();
            iceItem->RootComponent->SetRelativePosition(basePos + offset);
            iceItem->RootComponent->UpdateWorldTransform();

            //this->AddActor(sphereActor);
            this->AddActor(iceItem); 
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto iceItem = CreateShared<AMetalItem>();
            iceItem->RootComponent->SetRelativePosition(basePos + offset);
            iceItem->RootComponent->UpdateWorldTransform();

            //this->AddActor(sphereActor);
            this->AddActor(iceItem);
        }

    }



    //
     
    float step1 = 50.0f;
    Float3 end1{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)roadWidth, (uint32_t)step1);
        ApplySnow(actor);
        MarkAsFloor(actor);

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end1 = RotateAroundXAxis(actor, end0, step1, 20.0f);

        this->AddActor(actor);
    }


    float step2 = 500;
    Float3 end2{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)roadWidth, (uint32_t)step2);
        ApplySnow(actor);
        MarkAsFloor(actor);

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end2 = RotateAroundXAxis(actor, end1, step2, 0.0f);

        this->AddActor(actor);
    }


    //if(false)
    float gimmick1_0 = 10.0f;
    float gimmick1_0_e = 15.0f;
    {
        //new:
        Float3 dim = { 9,2,1 };
        auto& gridPattern = MMath::GenerateGrid3D(dim, { 1.01f, 1.01f, 1.01f });
        Float3 groupOffset = end1 + Float3(-dim.x() / 2, +0.5f, gimmick1_0);
        for (const auto& pos : gridPattern) {
            auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, pos + groupOffset);
            //auto actor = Mesh::CreateSphereActor(0.5f); 
            //actor->RootComponent->SetRelativePosition(pos + groupOffset);

            //generate random x y z rotation:
            //auto x = XMConvertToRadians(90.0f) * Random::Uniform01();  
            //auto y = XMConvertToRadians(90.0f) * Random::Uniform01();  
            //auto z = XMConvertToRadians(90.0f) * Random::Uniform01();   

            //auto rotation = XMQuaternionRotationRollPitchYaw(x, y, z);
            //actor->RootComponent->SetRelativeRotation(rotation);  

            actor->RootComponent->UpdateWorldTransform();

            auto& rb = actor->shapeComponent->rigidBody;
            rb->simulatePhysics = true;
            rb->simulateRotation = true;
            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

            this->AddActor(actor);
        }
    }


    float gimmick1_1 = gimmick1_0_e + 100.0f;
    float gimmick1_1_e{};
    {
        float height = 3.0f;
        float depth = 0.5f;
        float spacingZ = 20.0f;

        float width = roadWidth;

        gimmick1_1_e = gimmick1_1 + spacingZ * 10.0f;

        float sideOffset = 1.3;


        auto& gridPatternR = MMath::GenerateGrid3D({ 1,1,3 }, { 1.00f, 1.00f, spacingZ });
        Float3 groupOffsetR = end1 + Float3(width / 4 + sideOffset, height / 2, gimmick1_1);
        for (const auto& pos : gridPatternR) {
            auto actor = Mesh::CreateBoxActor(Float3{ width / 2, height, depth });
            //auto actor = Mesh::CreateSphereActor(0.5f); 
            actor->RootComponent->SetRelativePosition(pos + groupOffsetR);

            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

            this->AddActor(actor);
        }


        auto& gridPatternL = MMath::GenerateGrid3D({ 1,1,3 }, { 1.00f, 1.00f, spacingZ });
        Float3 groupOffsetL = end1 + Float3(-width / 4 - sideOffset, height / 2, gimmick1_1 + spacingZ / 2);
        for (const auto& pos : gridPatternL) {
            auto actor = Mesh::CreateBoxActor(Float3{ width / 2, height, depth });
            //auto actor = Mesh::CreateSphereActor(0.5f); 
            actor->RootComponent->SetRelativePosition(pos + groupOffsetL);

            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

            this->AddActor(actor);
        }
    }

 




    //
    {
        Float3 goalPos = end2 + Float3(0.0f, 0.0f, -0.2f);
        Float3 refShape = { roadWidth, 10.0f, 1.0f };

        auto checkPt = CreateShared<ATriggerVolume>();

        Mesh::SetBox(checkPt.get(), refShape);

        checkPt->RootComponent->SetRelativePosition(goalPos);
        checkPt->RootComponent->UpdateWorldTransform();


        this->AddActor(checkPt);
    }

}


void GamePlayLevel::LoadPlayer()
{
    //auto dftPlayer = CreateActor<APawn>(); 
    auto actor = CreateActor<APlayer>();

    actor->RootComponent->SetRelativePosition(playerSpawnPos); 
    actor->RootComponent->SetRelativeRotation(XMQuaternionRotationRollPitchYaw(0, MMath::ToRadians(-90.0f), 0));  
    actor->RootComponent->UpdateWorldTransform();

    //dftPlayer->staticMeshComponent->SetVisible(false);

    //auto possess the default player 
    if (auto controller = this->owningWorld->GetFirstPlayerController(); controller != nullptr) {
        std::cout << "level: default controller possess default default player\n";
        controller->Possess(actor);
    }

    this->AddActor(actor);
}


void GamePlayLevel::OnUnload()
{
    ULevel::OnUnload();

    std::cout << "unload game world" << '\n';

    //auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    //auto renderer = GameApplication::GetInstance()->GetRenderer();
    //renderer->ClearMesh();

    //owningWorld->physicsScene->ClearCollider();
    //owningWorld->physicsScene->ClearRigidBody();
    //owningWorld->physicsScene->ClearBuffer();

    //
    //for (auto& [id, actor] : m_staticMeshActors) {
    //    actor->onUpdate.Clear();
    //    // actor->onUpdate =
    //}

    //m_staticMeshActors.clear();
    //m_buttons.clear();
    //dummyCamera = nullptr;
}

void GamePlayLevel::OnTick(float delta)
{
    ULevel::OnTick(delta);

}

/*
void GamePlayWorld::GenerateObstacles(float roadWidth, float roadLength, uint32_t obstacleCount)
{

    auto renderer = GameApplication::GetInstance()->GetRenderer();

    std::random_device rd;
    std::mt19937 gen(rd());

    // Position ranges: X around center , Y height fixed, Z along the road from 0 to roadLength
    std::uniform_real_distribution<float> distX(-roadWidth, roadWidth);
    std::uniform_real_distribution<float> distZ(0.0f, roadLength);

    // Random choice between sphere or box
    std::uniform_int_distribution<int> shapeChoice(0, 1);

    for (uint32_t i = 0; i < obstacleCount; i++)
    {
        float x = distX(gen);
        float y = 1.0f; // height so above ground (adjust as needed)
        float z = distZ(gen);

        if (shapeChoice(gen) == 0)
        {
            // Create Sphere obstacle
            auto sphereProxy = CreateSphereActor({ x, y, z });
            sphereProxy->rigidBody->simulatePhysics = false;
            sphereProxy->rigidBody->simulateRotation = false;
            sphereProxy->rigidBody->material.friction = 5.0f;

            m_staticMeshActors.push_back(sphereProxy);
            //renderer->SubmitMesh(sphereProxy.get());

        }
        else
        {
            // Create Box obstacle with random size
            std::uniform_real_distribution<float> sizeDist(0.5f, 2.0f);
            float sizeX = sizeDist(gen);
            float sizeY = sizeDist(gen);
            float sizeZ = sizeDist(gen);

            auto boxProxy = CreateBoxActor({ x, y, z }, { sizeX, sizeY, sizeZ });
            boxProxy->rigidBody->simulatePhysics = false;
            boxProxy->rigidBody->simulateRotation = false;
            boxProxy->rigidBody->material.friction = 0.3f;


            m_staticMeshActors.push_back(boxProxy);
            //renderer->SubmitMesh(boxProxy.get());
        }
    }
}
*/