#include "PCH.h"
#include "DemoLevels.h"

#include "Application.h"

#include "Gameplay/World.h"
#include "Gameplay/Actors/StaticMeshActor.h"

#include "GameState.h"

#include "Item.h" 
#include "HUD.h"


using namespace DirectX;

void GamePlayLevel::OnLoad()
{
    ULevel::OnLoad();

    assert(owningWorld->physicsScene != nullptr);

    //new:
    //auto gameState = CreateShared<AGameState>(); 
    owningWorld->CreateGameState<AGameState>(); 
    auto gameState = owningWorld->GetGameState<AGameState>(); 

    //this->Load1();
    this->LoadActors();
    this->LoadUI();
}


void GamePlayLevel::LoadActors()
{ 
    //plane:
    auto planeActor = Mesh::CreatePlaneActor((uint32_t)roadWidth, (uint32_t)goalLength,
        Float3{ 0.0f, 0.0f, 0.0f }); 
    planeActor->staticMeshComponent->SetMaterial(Materials::GetSnowSurface());


    // 
    //auto sphereActor = Mesh::CreateSphereActor( 1.0f, Float3{ 2.0f, 2.0f, 0.0f });

    //auto& sphereRB = sphereActor->shapeComponent->rigidBody;
    //sphereRB->simulatePhysics = false;
    //sphereRB->simulateRotation = false; 
    //sphereActor->shapeComponent->SetIsTrigger(true); 
    //sphereActor->staticMeshComponent->SetMaterial(Materials::GetRustyIron()); 


    //auto itemActor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, { 0.0f, 2.0f, 4.0f }); 
    auto itemActor = CreateShared<ABoxItem>();
    itemActor->RootComponent->SetRelativePosition({ 0.0f, 1.0f, 4.0f }); 
    itemActor->RootComponent->UpdateWorldTransform(); 

    auto checkPt = CreateShared<ATriggerVolume>();
    checkPt->RootComponent->SetRelativePosition({ 0.0f, 1.0f, 8.0f });
    checkPt->RootComponent->UpdateWorldTransform();


    //this->AddActor(sphereActor);
    this->AddActor(planeActor);
    this->AddActor(itemActor);
    this->AddActor(checkPt);
}
 

void GamePlayLevel::LoadUI()
{
    auto playeHUD = CreateActor<APlayerHUD>();
    this->AddActor(playeHUD);

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
    std::cout << "unload game world" << '\n';

    //auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    renderer->ClearMesh();

    owningWorld->physicsScene->ClearCollider();
    owningWorld->physicsScene->ClearRigidBody();
    owningWorld->physicsScene->ClearBuffer();

    //
    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    renderer->ClearUI();
    uiManager->ClearRoot();

    //
    //for (auto& [id, actor] : m_staticMeshActors) {
    //    actor->onUpdate.Clear();
    //    // actor->onUpdate =
    //}

    //m_staticMeshActors.clear();
    m_buttons.clear();
    //dummyCamera = nullptr;
}

void GamePlayLevel::OnTick(float delta)
{
    ULevel::OnTick(delta); 

}

void GamePlayLevel::SyncGameToPhysics()
{
    //for (auto& [actorH, actor] : m_staticMeshActors) {
    //    owningWorld->physicsScene->SetPosition(actorH, actor->position);
    //    //owningWorld->physicsScene->SetRotation(actorH, actor->rotation); 
    //}

    //  for (auto& [actorH, rb] : owningWorld->physicsScene->m_bodies) {

    //      auto& owner = this->m_staticMeshActors[actorH];
    //      //::cout << "draw for rb:" << rb->debugName << '\n'; 
    //      if (!rb->simulatePhysics) continue;
          ////std::cout << "sync game to physics for rb: " << ToString(owner->position) << '\n';
    //      rb->position = owner->position;
    //      
    //      if (!rb->simulateRotation) continue;
    //      rb->rotation = owner->rotation;
    //  }
}

//void GamePlayWorld::SyncPhysicsToGame()
//{
//    //ULevel::SyncPhysicsToGame();
//
//    auto& transformBuffer = owningWorld->physicsScene->GetTransformBuffer();
//    for (auto& [actorH, trans] : transformBuffer) {
//
//        if (!this->m_staticMeshActors.contains(actorH)) continue;
//        auto& owner = this->m_staticMeshActors.at(actorH);
//
//        owner->position = trans.position;
//        owner->rotation = trans.rotation;
//    }
//
//    //for (auto& [actorH, rb] : owningWorld->physicsScene->m_bodies) {
//    //	 
//    //	auto& owner = this->m_staticMeshActors[actorH];
//    //	//::cout << "draw for rb:" << rb->debugName << '\n'; 
//    //	if (!rb->simulatePhysics) continue;
//    //	owner->SetWorldPosition(rb->position); 
//    // 
//    //	if (!rb->simulateRotation) continue;
//    //	owner->SetWorldRotation(rb->rotation);  
//    //
//    // 
//    //} 
//
//}




void MainMenuLevel::OnLoad()
{
    std::cout << "load main menu world" << '\n';

    auto gameManager = GameApplication::GetInstance()->GetGameStateManager();
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    int screenWidth = GameApplication::GetInstance()->GetWidth();
    int screenHeight = GameApplication::GetInstance()->GetHeight();

    //FRect buttonRect = { 0, 0, 500, 300 }; // Width and height of button
    FRect buttonRect = { 0, 0, 500, 50 }; // Width and height of button
    FRect centeredRect = CenterRect(screenWidth, screenHeight, buttonRect);

    auto entryButton = CreateShared<UIButton>(centeredRect);
    entryButton->text = "Press to Enter";

    auto click_cb = [=] {
        gameManager->RequestTransitState(GameStateId::Playing);
        };

    entryButton->OnClick.Add(click_cb);
    //debugButton->OnHover.Add(hover_cb); 

    FRect timeHUDRect = { centeredRect.x, centeredRect.y + 100, 500, 20 };
    auto timeHUD = CreateShared<UIButton>(timeHUDRect);
    timeHUD->text = std::format("Record :{:.2f}", Global::lastUsedTime);

    //todo:  manually submit
    //debugButton->Render(); 
    m_buttons.push_back(entryButton);
    m_buttons.push_back(timeHUD);
    uiManager->RegisterRootElement(entryButton.get());
    uiManager->RegisterRootElement(timeHUD.get());
}

void MainMenuLevel::OnUnload()
{
    std::cout << "unload main menu world" << '\n';

    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    assert(renderer != nullptr);

    renderer->ClearUI();

    uiManager->ClearRoot();

    m_buttons.clear();
}

void MainMenuLevel::OnTick(float delta)
{
    ULevel::OnTick(delta);
    //for (auto& button : m_buttons) {
    //    button->Tick(delta);
    //}
}


//void GamePlayWorld::SyncGameToPhysics()
//{
//    for (auto& [handle, proxy] : m_staticMeshActors) {
//        owningWorld->physicsScene->MoveRigidBody(handle, proxy->position);
//        owningWorld->physicsScene->RotateRigidBody(handle, proxy->rotation);
//    }
//}
//
//void GamePlayWorld::SyncPhysicsToGame()
//{
//    auto& readBuffer = owningWorld->physicsScene->syncBuffer.GetReadBuffer();
//	for (auto& [owner ,trans] : readBuffer) {
//		//auto& owner = trans.owner;
//        auto& actor = m_staticMeshActors[owner];
//		actor->SetWorldPosition(trans.position);
//		actor->SetWorldRotation(trans.rotation);
//	}
//}

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
