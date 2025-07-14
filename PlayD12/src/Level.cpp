#include "PCH.h"
#include "Level.h"

#include "Application.h"
#include "Render/Renderer.h"

#include "UI.h"

void GamePlayWorld::OnLoad()
{
    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    auto inputSystem = GameApplication::GetInstance()->GetInputSystem();
    auto gameManager = GameApplication::GetInstance()->GetGameStateManager();

    auto debugSphere = CreateShared<SphereMesh>();
    auto debugSphereProxy = renderer->InitMesh(debugSphere,
        { 0.0f, 7.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////new rigidbody for the sphere: todo
    auto rigidBodySphere0 = new RigidBody(debugSphereProxy, debugSphereProxy->position, Sphere{ debugSphereProxy->scale.x() }, debugSphereProxy->rotation);
    debugSphereProxy->rigidBody = rigidBodySphere0;
    physicsScene->AddRigidBody(rigidBodySphere0);
    auto sphereCollider0 = new Collider(debugSphereProxy, Sphere{ debugSphereProxy->scale.x() }, rigidBodySphere0); //sphere radius
    debugSphereProxy->collider = sphereCollider0;
    physicsScene->AddCollider(sphereCollider0);

    rigidBodySphere0->debugName = "Sphere";
    rigidBodySphere0->simulateRotation = true;





    //---------------
    auto plane0 = CreateShared<PlaneMesh>();
    auto planeProxy = renderer->InitMesh(plane0,
        { 0.0f, -4.0f, 0.0f },
        { 10.0f, 20.0f, 50.0f }
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
            rigidBodySphere0->ApplyForce(FLOAT3(0.0f, 0.0f, +10.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::S)) {
            rigidBodySphere0->ApplyForce(FLOAT3(0.0f, 0.0f, -10.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::A)) {
            rigidBodySphere0->ApplyForce(FLOAT3(-10.0f, 0.0f, 0.0f));
        }
        if (inputSystem->IsKeyDown(KeyCode::D)) {
            rigidBodySphere0->ApplyForce(FLOAT3(+10.0f, 0.0f, 0.0f));
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