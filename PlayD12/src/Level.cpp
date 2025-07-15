#include "PCH.h"
#include "Level.h"

#include "Application.h"
#include "Render/Renderer.h"

#include "UI.h"

using namespace DirectX;

void GamePlayWorld::OnLoad()
{
    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    auto inputSystem = GameApplication::GetInstance()->GetInputSystem();
    auto gameManager = GameApplication::GetInstance()->GetGameStateManager();

    auto debugSphere = CreateShared<SphereMesh>();
    auto debugSphereProxy = renderer->InitMesh(debugSphere,
        { 0.0f, 2.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////new rigidbody for the sphere: todo
    auto rigidBodySphere0 = new RigidBody(debugSphereProxy, debugSphereProxy->position, Sphere{ debugSphereProxy->scale.x() }, debugSphereProxy->rotation);
    debugSphereProxy->rigidBody = rigidBodySphere0;
    physicsScene->AddRigidBody(rigidBodySphere0);
    auto sphereCollider0 = new Collider(debugSphereProxy, Sphere{ debugSphereProxy->scale.x() }, rigidBodySphere0); //sphere radius
    debugSphereProxy->collider = sphereCollider0;
    physicsScene->AddCollider(sphereCollider0);

    rigidBodySphere0->debugName = "DebugSphere";
    rigidBodySphere0->simulateRotation = true;

    //rigidBodySphere0->angularVelocity = FLOAT3{ 1.0f, 1.0f, 1.0f };

     
    //-------------------------
    auto cubeMesh0 = CreateShared<CubeMesh>();
    auto cubeProxy0 = renderer->InitMesh(cubeMesh0,
        { 3.0f, 3.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////a initial rotation of the cube: 
    //auto rotation2 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(45.0f));
    //cubeProxy0->rotation = XMQuaternionMultiply(rotation2, cubeProxy0->rotation);

    ////new rigidbody for the cube:
    auto cubeRB0 = new RigidBody(cubeProxy0, cubeProxy0->position, Box{ cubeProxy0->scale }, cubeProxy0->rotation);
    cubeProxy0->rigidBody = cubeRB0;
    physicsScene->AddRigidBody(cubeRB0);
    auto cubeCollider0 = new Collider(cubeProxy0, Box{ cubeProxy0->scale }, cubeRB0); //half extents
    cubeProxy0->collider = cubeCollider0;
    physicsScene->AddCollider(cubeCollider0);

    cubeRB0->simulateRotation = false;
    cubeRB0->debugName = "debugCube"; 
     
	cubeRB0->material.friction = 0.0f;  

    //---------------
    auto plane0 = CreateShared<PlaneMesh>();
    auto planeProxy = renderer->InitMesh(plane0,
        { 0.0f, 0.0f, 0.0f },
        { 50.0f, 50.0f, 50.0f }
    );

    //add a rigidbody for the plane:
    auto rigidBodyPlane = new RigidBody(planeProxy, planeProxy->position, Plane{ planeProxy->scale.x(), planeProxy->scale.z() }, planeProxy->rotation);
    planeProxy->rigidBody = rigidBodyPlane;
    physicsScene->AddRigidBody(rigidBodyPlane);

    rigidBodyPlane->mass = MMath::FLOAT_MAX;
    rigidBodyPlane->simulatePhysics = false;

    auto planeCollider = new Collider(planeProxy, Plane{ planeProxy->scale.x(), planeProxy->scale.z() }, rigidBodyPlane);
    planeProxy->collider = planeCollider;
    physicsScene->AddCollider(planeCollider);

    rigidBodyPlane->simulateRotation = false;
    rigidBodyPlane->debugName = "DebugPlane";


 



    auto debugBehavior = [=](float delta) {
        //std::cout << "tick behavior" << '\n';
        if (inputSystem == nullptr) {
            std::cerr << "empty input system" << '\n';
            return;
        }
        if (inputSystem->IsKeyDown(KeyCode::W)) {
            rigidBodySphere0->ApplyForce(FLOAT3(0.0f, 0.0f, +2.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::S)) {
            rigidBodySphere0->ApplyForce(FLOAT3(0.0f, 0.0f, -2.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::A)) {
            rigidBodySphere0->ApplyForce(FLOAT3(-2.0f, 0.0f, 0.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::D)) {
            rigidBodySphere0->ApplyForce(FLOAT3(+2.0f, 0.0f, 0.0f));
        }

        if (rigidBodySphere0->position.y() < rigidBodyPlane->position.y()) {
            std::cout << "game over? " << '\n';
            gameManager->TransitState(GameStateId::MainMenu);
        }

        };

    debugSphereProxy->onUpdate.Add(debugBehavior);

    //---------------
    renderer->SubmitMesh(debugSphereProxy);
    renderer->SubmitMesh(planeProxy);

    renderer->SubmitMesh(cubeProxy0);
}

void GamePlayWorld::OnUnload()
{
    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    renderer->ClearMesh();
    physicsScene->ClearCollider();
    physicsScene->ClearRigidBody();

    std::cout << "unload game world" << '\n';
}

void GamePlayWorld::OnUpdate(float delta)
{
}

void MainMenuWorld::OnLoad()
{
    auto gameManager = GameApplication::GetInstance()->GetGameStateManager();
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    Rect rect = { 200,200,500,500 };
    debugButton = CreateShared<UIButton>(rect);

    auto click_cb = [=] {
        gameManager->TransitState(GameStateId::Playing);
        };

    debugButton->OnClick.Add(click_cb);

    //todo:  manually submit
    //debugButton->Render(); 
    uiManager->RegisterRootElement(debugButton.get());
}

void MainMenuWorld::OnUnload()
{
    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    auto uiRenderer = GameApplication::GetInstance()->GetRenderer()->uiRenderer;
    assert(uiRenderer != nullptr);

    uiRenderer->Clear();

    uiManager->ClearRoot();
}

void MainMenuWorld::OnUpdate(float delta)
{
    debugButton->Tick(delta);
}