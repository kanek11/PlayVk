#include "PCH.h"
#include "DemoLevels.h"

#include "Application.h"

#include "Gameplay/World.h"
#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"

#include "Item.h" 
//#include "HUD.h"

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
    this->LoadActors();
    this->LoadUI();
}


void GamePlayLevel::LoadActors()
{
    //plane:
    auto planeActor = Mesh::CreatePlaneActor((uint32_t)roadWidth, (uint32_t)goalLength,
        Float3{ 0.0f, 0.0f, 0.0f });
    //planeActor->staticMeshComponent->SetMaterial(Materials::GetSnowSurface());

    // 
    //auto sphereActor = Mesh::CreateSphereActor( 1.0f, Float3{ 2.0f, 2.0f, 0.0f });

    //auto& sphereRB = sphereActor->shapeComponent->rigidBody;
    //sphereRB->simulatePhysics = false;
    //sphereRB->simulateRotation = false; 
    //sphereActor->shapeComponent->SetIsTrigger(true); 
    //sphereActor->staticMeshComponent->SetMaterial(Materials::GetRustyIron()); 


    //auto itemActor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, { 0.0f, 2.0f, 4.0f }); 
    //auto itemActor = CreateShared<ABoxItem>();
    //itemActor->RootComponent->SetRelativePosition({ 0.0f, 1.0f, 4.0f });
    //itemActor->RootComponent->UpdateWorldTransform();

    //auto checkPt = CreateShared<ATriggerVolume>();
    //checkPt->RootComponent->SetRelativePosition({ 0.0f, 1.0f, 2.0f });
    //checkPt->RootComponent->UpdateWorldTransform();


    //this->AddActor(sphereActor);
    this->AddActor(planeActor);
    //this->AddActor(itemActor);
    //this->AddActor(checkPt);


    //new:
    auto& gridPattern = MMath::GenerateGrid3D({ 3,3,1 }, 1.1f);
    for (const auto& pos : gridPattern) {
        auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, pos);
		//auto actor = Mesh::CreateSphereActor(0.5f, pos);
        actor->RootComponent->SetRelativePosition(pos);

        //rotate 30 degree:
        //generate random x y z rotation:
        //auto x = XMConvertToRadians(90.0f) * Random::Uniform01();  
        //auto y = XMConvertToRadians(90.0f) * Random::Uniform01();  
        //auto z = XMConvertToRadians(90.0f) * Random::Uniform01();  

        auto x = XMConvertToRadians(90.0f);  
        auto y = XMConvertToRadians(0.0f);  
        auto z = XMConvertToRadians(90.0f);  

        auto rotation = XMQuaternionRotationRollPitchYaw(x, y, z);
        actor->RootComponent->SetRelativeRotation(rotation);  

        actor->RootComponent->UpdateWorldTransform();

        auto& rb = actor->shapeComponent->rigidBody;
        rb->simulatePhysics = true;
        rb->simulateRotation = true;
        //actor->shapeComponent->SetIsTrigger(true); 
        //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

        this->AddActor(actor);
    }


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


void GamePlayLevel::LoadPlayer()
{
    //auto dftPlayer = CreateActor<APawn>(); 
    auto dftPlayer = CreateActor<APlayer>();

    dftPlayer->RootComponent->SetRelativePosition({ 0.0f, 2.0f, -4.0f });
    dftPlayer->RootComponent->UpdateWorldTransform();

    dftPlayer->staticMeshComponent->SetVisible(false);

    //auto possess the default player 
    if (auto controller = this->owningWorld->GetFirstPlayerController(); controller != nullptr) {
        std::cout << "level: default controller possess default default player\n";
        controller->Possess(dftPlayer);
    }

    this->AddActor(dftPlayer);
}


void GamePlayLevel::LoadUI()
{
    //auto playeHUD = CreateActor<APlayerHUD>();
    //this->AddActor(playeHUD);

    //auto uiManager = GameApplication::GetInstance()->GetUIManager();

    ////FRect buttonRect = { 0, 0, 300, 150 };
    //FRect buttonRect = { 10, 20, 300, 50 };
    //auto debugHUD = CreateShared<UIButton>(buttonRect);

    //FRect buttonRect1 = { 10, 100, 300, 50 };
    //auto debugHUD1 = CreateShared<UIButton>(buttonRect1);

    //FRect buttonRect2 = { 10, 200, 300, 50 };
    //auto debugHUD2 = CreateShared<UIButton>(buttonRect2);

    //m_buttons.push_back(debugHUD);
    //m_buttons.push_back(debugHUD1);
    //m_buttons.push_back(debugHUD2);

    ////todo:  manually submit 
    //uiManager->RegisterRootElement(debugHUD.get());
    //uiManager->RegisterRootElement(debugHUD1.get());
    //uiManager->RegisterRootElement(debugHUD2.get());
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
