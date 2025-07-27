#include "PCH.h"
#include "Level.h"

#include "Application.h"
#include "Render/Renderer.h"

using namespace DirectX;

#include <random>
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
            renderer->SubmitMesh(sphereProxy.get());

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
            renderer->SubmitMesh(boxProxy.get());
        }
    }
}


void GamePlayWorld::OnLoad()
{
    //hardcode state; todo;
    timeCount = 0.0f;

    std::cout << "load game world" << '\n';

    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    auto inputSystem = GameApplication::GetInstance()->GetInputSystem();
    auto gameManager = GameApplication::GetInstance()->GetGameStateManager();


    int screenWidth = GameApplication::GetInstance()->GetWidth();
    int screenHeight = GameApplication::GetInstance()->GetHeight();


    auto debugSphereProxy = CreateSphereActor({ 0.0f, 1.0f, 0.0f });
    debugSphereProxy->rigidBody->simulatePhysics = true;
    debugSphereProxy->rigidBody->simulateRotation = true;

    debugSphereProxy->rigidBody->debugName = "debug sphere";
    debugSphereProxy->rigidBody->material.friction = 10.0f;

    //------------------------- 
    auto debugCubeProxy = CreateBoxActor({ 3.0f, 3.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
    debugCubeProxy->rigidBody->simulatePhysics = true;
    debugCubeProxy->rigidBody->simulateRotation = false;
    debugCubeProxy->rigidBody->material.friction = 0.1f;

    //--------------- 
    auto planeProxy = CreatePlaneActor({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, static_cast<uint32_t>(roadWidth), static_cast<uint32_t>(goalLength));

    //auto debugPlayer = debugCubeProxy;
    auto debugPlayer = debugSphereProxy;


    //---------------
    //this->GenerateObstacles(roadWidth / 2, goalLength, 30);


    //---------------
    //renderer->SubmitMesh(debugSphereProxy.get());
    //renderer->SubmitMesh(planeProxy.get());
    //renderer->SubmitMesh(debugCubeProxy.get());


    m_staticMeshActors.push_back(debugSphereProxy);
    m_staticMeshActors.push_back(planeProxy);
    m_staticMeshActors.push_back(debugCubeProxy);

    //
    dummyCamera = new FollowCameraProxy();
    dummyCamera->target = debugPlayer;

    renderer->SubmitCamera(dummyCamera);


    // 
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    //FRect buttonRect = { 0, 0, 300, 150 };
    FRect buttonRect = { 10, 20, 300, 50 };
    auto debugHUD = CreateShared<UIButton>(buttonRect); 

    FRect buttonRect1 = { 10, 100, 300, 50 };
    auto debugHUD1 = CreateShared<UIButton>(buttonRect1);

    FRect buttonRect2 = { 10, 200, 300, 50 };
    auto debugHUD2 = CreateShared<UIButton>(buttonRect2);


    m_HUDs.push_back(debugHUD);
    m_HUDs.push_back(debugHUD1);
    m_HUDs.push_back(debugHUD2);

    //todo:  manually submit 
    uiManager->RegisterRootElement(debugHUD.get());
    uiManager->RegisterRootElement(debugHUD1.get());
    uiManager->RegisterRootElement(debugHUD2.get());
     

    //--------------------------


    auto debugBehavior = [=](float delta) {
        //std::cout << "tick behavior" << '\n';
        //std::cout << "debugSphereProxy speed:" << ToString(debugSphereProxy->rigidBody->linearVelocity) << '\n';

        float currSpeed = Length(debugPlayer->rigidBody->linearVelocity);
        debugHUD->text = std::format("vel:{:.2f}", currSpeed);

        float currDist = goalLength / 2 - debugPlayer->position.z();
        debugHUD1->text = std::format("dist:{:.2f}", currDist);

        timeCount += delta;
        debugHUD2->text = std::format("time:{:.2f}", timeCount);

        if (inputSystem == nullptr) {
            std::cerr << "empty input system" << '\n';
            return;
        }
        //if (inputSystem->IsKeyDown(KeyCode::W)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(0.0f, 0.0f, +5.0f));
        //}
        //if (inputSystem->IsKeyDown(KeyCode::S)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(0.0f, 0.0f, -5.0f));
        //}
        //if (inputSystem->IsKeyDown(KeyCode::S)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(0.0f, 0.0f, -5.0f));
        //}
        //if (inputSystem->IsKeyDown(KeyCode::A)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(-5.0f, 0.0f, 0.0f));
        //}
        float axisZ = inputSystem->GetAxis(EAxis::MoveZ);
        debugPlayer->rigidBody->ApplyForce(Float3(0.0f, 0.0f, axisZ * 5.0f));

        float axisX = inputSystem->GetAxis(EAxis::MoveX);
        debugPlayer->rigidBody->ApplyForce(Float3(axisX * 5.0f, 0.0f, 0.0f));


        //if (inputSystem->IsKeyDown(KeyCode::D)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(+5.0f, 0.0f, 0.0f));
        //}

        //if (inputSystem->IsKeyDown(KeyCode::E)) {
        //    debugPlayer->rigidBody->ApplyForce(Float3(0.0f, +30.0f, 0.0f));
        //}

        if (debugPlayer->position.y() < planeProxy->position.y()) {
            std::cout << "game over? " << '\n';
            debugPlayer->position = { 0.0f, 2.0f, 0.0f };
            debugPlayer->rigidBody->linearVelocity = Float3{};
            debugPlayer->rigidBody->angularVelocity = Float3{};

            timeCount = 0.0f;
            //gameManager->RequestTransitState(GameStateId::MainMenu);
        }

        else if (debugPlayer->position.z() > goalLength / 2 - 20) {
            //std::cout << "goal?" << '\n';
            //if (timeCount < Global::lastUsedTime)
            //    Global::lastUsedTime = timeCount;
            //gameManager->RequestTransitState(GameStateId::MainMenu);
        }

        };

    debugPlayer->onUpdate.Add(debugBehavior);

}

void GamePlayWorld::OnUnload()
{
    std::cout << "unload game world" << '\n';


    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    renderer->ClearMesh();
    physicsScene->ClearCollider();
    physicsScene->ClearRigidBody();

    //
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    renderer->ClearUI();
    uiManager->ClearRoot();

    //
    m_staticMeshActors.clear();
    m_HUDs.clear();
    dummyCamera = nullptr;
}

void GamePlayWorld::OnUpdate(float delta)
{

    DebugDraw::Get().AddRay(
        Float3(0.0f, 0.0f, 0.0f),
        Float3(1.0f, 0.0f, 0.0f),
        Color::Red
    );

    DebugDraw::Get().AddRay(
        Float3(0.0f, 0.0f, 0.0f),
        Float3(0.0f, 1.0f, 0.0f),
        Color::Green
    );


    DebugDraw::Get().AddRay(
        Float3(0.0f, 0.0f, 0.0f),
        Float3(0.0f, 0.0f, 1.0f),
        Color::Blue
    );


    //--------------
    for (auto& HUD : m_HUDs) {
        HUD->Tick(delta);
    }


    //--------------
    //m_debugRenderer->OnUpdate(delta, dummyCamera.pvMatrix); 
    {

        dummyCamera->Tick(delta);
    }


    //--------------
    for (auto& proxy : m_staticMeshActors)
    {
        proxy->onUpdate.BlockingBroadCast(delta);

        //auto objectCBH = proxy->objectCB;
        //if (objectCBH == nullptr) {
        //    continue;
        //}

        //auto yAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        //auto rotation = XMQuaternionRotationAxis(yAxis, delta);
        //proxy->rotation = XMQuaternionMultiply(rotation, proxy->rotation);

        //ObjectCB objectCBData = {};

        auto modelMatrix_ = MMath::MatrixIdentity<float, 4>();
        auto scaleMatrix = MMath::MatrixScaling(proxy->scale.x(), proxy->scale.y(), proxy->scale.z());
        modelMatrix_ = MatrixMultiply(scaleMatrix, modelMatrix_);

        auto R_ = XMMatrixRotationQuaternion(proxy->rotation);
        auto R = MMath::MatrixIdentity<float, 4>();
        R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] , 0.0f };
        R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] , 0.0f };
        R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] , 0.0f };
        R[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

        modelMatrix_ = MatrixMultiply(R, modelMatrix_); //rotate the model using the quaternion 

        //translate: 
        auto translation = MMath::MatrixTranslation(proxy->position.x(), proxy->position.y(), proxy->position.z());
        //translation = Transpose(translation); 
        //std::cout << "translation matrix: " << ToString(translation) << std::endl;

        //modelMatrix_ = MatrixMultiply(translation, modelMatrix_); 

        modelMatrix_ = MatrixMultiply(translation, modelMatrix_);

        //modelMatrix_ = MMath::MatrixIdentity<float, 4>();
        //modelMatrix_ = Transpose(modelMatrix_); 

        //auto modelMatrix = XMMatrixIdentity();
        //auto translation_ = XMMatrixTranslation(proxy->position.x(), proxy->position.y(), proxy->position.z());
        //std::cout << "expected translation: " << '\n';
        //std::cout << MMath::XMMatrixToString(translation_) << std::endl;
        //

        //modelMatrix = XMMatrixRotationQuaternion(proxy->rotation) * modelMatrix;
        //modelMatrix = XMMatrixScaling(proxy->scale.x(), proxy->scale.y(), proxy->scale.z()) * modelMatrix;

        // Translate the model to its position
        // Rotate the model using the quaternion 

        //XMStoreFloat4x4(&mainConstBufferData.modelMatrix, modelMatrix);

        //make sure transpose before present to hlsl;
        //mainConstBufferData.modelMatrix = Transpose(modelMatrix_);

        //objectCBData.modelMatrix = modelMatrix_;

        //XMStoreFloat4x4(&constBufferData.viewProjectionMatrix, vp);
        //mainConstBufferData.projectionViewMatrix = dummyCamera->pvMatrix;

        // Upload the constant buffer data.
        //objectCBH->UploadData(&objectCBData, sizeof(ObjectCB));

        proxy->modelMatrix = modelMatrix_;



        //shadow pass:
        //auto shadowConstBufferH = proxy->shadowMVPConstantBuffer;
        //if (shadowConstBufferH == nullptr) {
        //    continue;
        //}

        //MVPConstantBuffer shadowConstBufferData = {};
        //shadowConstBufferData.modelMatrix = modelMatrix_;
        //shadowConstBufferData.projectionViewMatrix = dummyCamera->pvMatrix;

        //// Upload the shadow constant buffer data.
        //shadowConstBufferH->UploadData(&shadowConstBufferData, sizeof(MVPConstantBuffer));


        DebugDraw::Get().AddRay(
            proxy->position,
            R[0].xyz(),
            Color::Red
        );

        DebugDraw::Get().AddRay(
            proxy->position,
            R[1].xyz(),
            Color::Green
        );

        DebugDraw::Get().AddRay(
            proxy->position,
            R[2].xyz(),
            Color::Blue
        );

    }


    //--------------
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    for (auto& proxy : m_staticMeshActors) {

        renderer->SubmitMesh(proxy.get());
    }


}





void MainMenuWorld::OnLoad()
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
    m_Buttons.push_back(entryButton);
    m_Buttons.push_back(timeHUD);
    uiManager->RegisterRootElement(entryButton.get());
    uiManager->RegisterRootElement(timeHUD.get());
}

void MainMenuWorld::OnUnload()
{
    std::cout << "unload main menu world" << '\n';

    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    assert(renderer != nullptr);

    renderer->ClearUI();

    uiManager->ClearRoot();


    m_Buttons.clear();
}

void MainMenuWorld::OnUpdate(float delta)
{
    for (auto& button : m_Buttons) {
        button->Tick(delta);
    }
}