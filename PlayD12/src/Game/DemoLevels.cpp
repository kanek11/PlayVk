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


*/


void MarkAsFloor(AStaticMeshActor* actor) {
    actor->tag = "floor";
    actor->shapeComponent->collider->bNeedsEvent = true;
}

void ApplySnow(AStaticMeshActor* actor) {

    //actor->staticMeshComponent->SetMaterial(Materials::GetSnowSurface());

    actor->shapeComponent->rigidBody->SetPhysicalMaterial(PhysicalMaterial{ 0.0f, 0.0f });
}

void DisablePhysics(AStaticMeshActor* actor) {
	actor->shapeComponent->SetSimulatePhysics(false);
	//actor->shapeComponent->SetCollisionEnabled(false);
}

void ApplyHighFriction(AStaticMeshActor* actor) {
	auto& rb = actor->shapeComponent->rigidBody;
	rb->simulatePhysics = true;
	rb->simulateRotation = true;

    rb->linearDamping = 0.999f;
	rb->angularDamping = 0.999f;

    rb->compliance = 0.0001f;
	rb->SetPhysicalMaterial(PhysicalMaterial{ 0.0f, 0.5f });
     

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
    this->LoadMap0();  
    this->LoadPlayer();
}

 

static Float3 RotateAroundXAxis(AActor* actor, Float3 pivot, float radius, float angle) {

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
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step0);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end0 = RotateAroundXAxis(actor.get(), {0.f,0.f,0.f}, step0, 0.0f);

        this->AddActor(actor);
    }

    // 
    float step1 = 100.0f;
    Float3 end1_1{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step1);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end1_1 = RotateAroundXAxis(actor.get(), end0, step1, 20.0f);

        this->AddActor(actor);
    }

    //
    float step2 = 200.0f;
    Float3 end1_2{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step2);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end1_2 = RotateAroundXAxis(actor.get(), end1_1, step2, 0.0f);

        this->AddActor(actor);
    }

    float step3 = 100;
    Float3 end2_1{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step3);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end2_1 = RotateAroundXAxis(actor.get(), end1_2, step3, 20.0f);

        this->AddActor(actor);
    }

    float step4 = 150;
    Float3 end2_2{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step4);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end2_2 = RotateAroundXAxis(actor.get(), end2_1, step4, 0.0f);

        this->AddActor(actor);
    }


    float step5 = 100;
    Float3 end3_1{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step5);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end3_1 = RotateAroundXAxis(actor.get(), end2_2, step5, 20.0f);

        this->AddActor(actor);
    }


    float step6 = 200;
    Float3 end3_2{};
    {
        auto actor = Mesh::CreatePlaneActor((uint32_t)LANE_WIDTH, (uint32_t)step6);
        ApplySnow(actor.get());
        MarkAsFloor(actor.get());

        //actor->RootComponent->SetRelativePosition(Float3{ 0.0f, 0.0f, step0 / 2 });
        end3_2 = RotateAroundXAxis(actor.get(), end3_1, step6, 0.0f);

        this->AddActor(actor);
    }


    //level 0================= 
    {
        auto& grid = MMath::GenerateGrid3D({ 1,1,3 }, { 1,1,4.0f });
        Float3 offset = { 0.0 , 1.0f, 6.0f };
        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<ACloneItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();
             
            this->AddActor(item);
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AMetalItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();
             
            this->AddActor(item);
        }
         
        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AIceItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();
             
            this->AddActor(item);
        }

    }



    //level 1=================  

    //if(false)
    float gimmick1_0 = 10.0f;
    float gimmick1_0_e = 15.0f;
    {
        //new:
        Float3 dim = { 9,2,1 };
        auto& gridPattern = MMath::GenerateGrid3D(dim, { 1.01f, 1.01f, 1.01f });
        Float3 groupOffset = end1_1 + Float3(-dim.x() / 2, +0.5f, gimmick1_0);
        for (const auto& pos : gridPattern) {
            auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f }, pos + groupOffset);
 

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

            rb->linearDamping = 0.98f;
            rb->angularDamping = 0.98f;

            rb->compliance = 0.0001f;

            rb->bFastStable = true;

            this->AddActor(actor);
        }
    }


    float gimmick1_1 = gimmick1_0_e + 20.0f;
    float gimmick1_1_e{};
    {
        float height = 3.0f;
        float depth = 0.5f;
        float spacingZ = 20.0f;

        float width = LANE_WIDTH;

        gimmick1_1_e = gimmick1_1 + spacingZ * 4.0f;

        float sideOffset = 1.3f;


        auto& gridPatternR = MMath::GenerateGrid3D({ 1,1,3 }, { 1.00f, 1.00f, spacingZ });
        Float3 groupOffsetR = end1_1 + Float3(width / 4 + sideOffset, height / 2, gimmick1_1);
        for (const auto& pos : gridPatternR) {
            auto actor = Mesh::CreateBoxActor(Float3{ width / 2, height, depth });
            //auto actor = Mesh::CreateSphereActor(0.5f); 
            actor->RootComponent->SetRelativePosition(pos + groupOffsetR);

            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

            this->AddActor(actor);
        }


        auto& gridPatternL = MMath::GenerateGrid3D({ 1,1,3 }, { 1.00f, 1.00f, spacingZ });
        Float3 groupOffsetL = end1_1 + Float3(-width / 4 - sideOffset, height / 2, gimmick1_1 + spacingZ / 2);
        for (const auto& pos : gridPatternL) {
            auto actor = Mesh::CreateBoxActor(Float3{ width / 2, height, depth });
            //auto actor = Mesh::CreateSphereActor(0.5f); 
            actor->RootComponent->SetRelativePosition(pos + groupOffsetL);

            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

            this->AddActor(actor);
        }
    }


    float gimmick1_2 = gimmick1_1_e + 5.0f;
    float gimmick1_2_e{};
    {
        float spacing = 20.0f;
        auto& grid = MMath::GenerateGrid3D({ 1,1,2 }, { 1,1, spacing });
		int index = 0;
		for (const auto& pos : grid) {

            float height = 0.8f;
            float width = LANE_WIDTH * 0.6f;

            float amplitude = 0.0f;

            Float3 refPos = end1_1 + Float3(0.0f, height / 2 + amplitude, gimmick1_2  + pos.z());//Float3(0.0f, height, gimmick1_1_e);
            Float3 refShape = { width, height, 1.0f };

            auto actor = CreateActor<AOscillateBox>();
            actor->m_amplitude = amplitude;
			actor->m_phase = (index % 3) * MMath::PI / 3;

            Mesh::SetBox(actor.get(), refShape);

            actor->RootComponent->SetRelativePosition(refPos);
            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

            this->AddActor(actor);
			index++;
		}
      

    }

    {
        float spacing = 3.0f;
        auto& grid = MMath::GenerateGrid3D({ 3,1,1 }, { spacing, 1,1 });
        Float3 offset = end1_2 + Float3(- spacing * 1.0f, 1.0f, 0.0f);
        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<ACloneItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AIceItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AMetalItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }


    }

     

    //============================
    float gimmick_2_0 = 5.0f;
    float gimmick2_0_e{};
    {
        
        Float3 dim = { 3,1,3 };
        float spacing = 4.0f;
        auto& gridPattern = MMath::GenerateGrid3D(dim, { spacing, spacing, spacing });
        Float3 groupOffset =  end2_1 + Float3(-dim.x() / 2 * (spacing + 1.0f), +1.0f, gimmick_2_0);

		gimmick2_0_e = gimmick_2_0 + dim.z() * (spacing + 1.0f);

        for (const auto& pos : gridPattern) { 

			auto targetPos = pos + groupOffset + Float3(Random::Uniform01() * 2.0f -1.0f, 0.0f, Random::Uniform01() * 2.0f - 1.0f) * spacing * 0.8f;
            auto actor = Mesh::CreateSphereActor(1.0f, pos + targetPos);
            actor->RootComponent->UpdateWorldTransform();

            auto& rb = actor->shapeComponent->rigidBody;
            rb->simulatePhysics = true;
            rb->simulateRotation = true;
            //actor->staticMeshComponent->SetMaterial(Materials::GetIron());  
             
			ApplyHighFriction(actor.get());

            //rb->compliance = 0.0001f;

            //rb->bFastStable = true;

            this->AddActor(actor);
        }
    }


	float gimmick2_1 = gimmick2_0_e + 10.0f;
    float gimmick2_1_e{};

    { 
        float height = 1.0f;
        float width = 3.0f;
		float length = 4.0f;

		auto& grid = MMath::GenerateGrid3D({ 3,1,1 }, { width * 2,1,1 }); 
		auto groupOffset = Float3(-width * 2.f, +height - 0.4f, gimmick2_1);

		gimmick2_1_e = gimmick2_1 + length;

		for (const auto& pos : grid) {
             
            Float3 refPos = pos + groupOffset +  end2_1;
            Float3 refShape = { width, height, length };

            auto actor = Mesh::CreateBoxActor(refShape);

			actor->RootComponent->SetRelativePosition(refPos);
            //rotate 30:
			auto rotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(-30.0f),0.0f , 0.0f);
			actor->RootComponent->SetRelativeRotation(rotation); 

            actor->RootComponent->UpdateWorldTransform(); 

            this->AddActor(actor);
		}
  
    }

     
    float gimmick2_2 = gimmick2_1_e + 5.0f;
    float gimmick2_2_e{};
    {
        float spacing = 10.0f;
        auto& grid = MMath::GenerateGrid3D({ 1,1,3 }, { 1,1, spacing });
        int index = 0;

        float height = 2.0f;

		auto groupOffset = Float3(0.0f, height/2, gimmick2_2);

		gimmick2_2_e = gimmick2_2 + spacing * 3.0f;

        for (const auto& pos : grid) {

            float width = LANE_WIDTH * 0.6f;

            float amplitude = 1.0f;

            Float3 refPos = end2_1 + groupOffset + pos; 
            Float3 refShape = { width, height, 1.0f };

            auto actor = CreateActor<AOscillateBox>();
            actor->m_amplitude = amplitude;
            actor->m_phase = (index % 3) * MMath::PI / 3;

            Mesh::SetBox(actor.get(), refShape);

            actor->RootComponent->SetRelativePosition(refPos);
            actor->RootComponent->UpdateWorldTransform();

            //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

            this->AddActor(actor);
            index++;
        }


    }


	float gimmick2_3 = gimmick2_2_e + 20.0f;
	float gimmick2_3_e{};
    {
        float width = 3.0f;
        float height = 3.0f;
        float depth = 0.2f;
        float spacingZ = 4.0f; 

		float rotationSpeed = 30.0f;

        gimmick2_3_e = gimmick2_3 + spacingZ * 4.0f;

        auto& gridPatternR = MMath::GenerateGrid3D({ 4,1,4 }, { spacingZ, 1, spacingZ });
        Float3 groupOffsetR = end2_1 + Float3(-(width * 0.5f + spacingZ ), height / 2, gimmick2_3);
         
        Float3 dim = { width, height, depth };

        for (const auto& pos : gridPatternR) {

            {
                auto targetPos = pos + groupOffsetR;

                auto actor = CreateActor<ARotateBox>();
                Mesh::SetBox(actor.get(), dim);
                actor->SetRotationSpeed(rotationSpeed);

                actor->RootComponent->SetRelativePosition(targetPos);
                actor->RootComponent->UpdateWorldTransform();

                //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

                this->AddActor(actor);
            }

            {
                auto targetPos = pos + groupOffsetR;

                auto actor = CreateActor<ARotateBox>();
                Mesh::SetBox(actor.get(), dim);
                actor->SetRotationSpeed(rotationSpeed);

                actor->RootComponent->SetRelativePosition(targetPos);
                auto rotation = XMQuaternionRotationRollPitchYaw(0.0f, XMConvertToRadians(90.0f), 0.0f);
                actor->RootComponent->SetRelativeRotation(rotation); 
                actor->RootComponent->UpdateWorldTransform();

                //actor->staticMeshComponent->SetMaterial(Materials::GetIron());

                this->AddActor(actor);
            }


        } 
    }


    
    {
        float spacing = 3.0f;
        auto& grid = MMath::GenerateGrid3D({ 3,1,1 }, { spacing, 1,1 });
        Float3 offset = end2_2 + Float3(-spacing * 1.0f, 1.0f, 0.0f);
        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<ACloneItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AIceItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }

        {
            Float3 basePos = grid.back();
            grid.pop_back();

            auto item = CreateShared<AMetalItem>();
            item->RootComponent->SetRelativePosition(basePos + offset);
            item->RootComponent->UpdateWorldTransform();

            this->AddActor(item);
        }


    }


    //============================  
	float gimmick3_0 = 10.0f;
    {

        Float3 dim = { 6,2,3 };

		float height = 1.0f;
        float depth = 1.f;
		float width = 1.0;
		Float3 refShape = { width, height, depth };

		float spacingZ = 10.0f;

        auto& gridPattern = MMath::GenerateGrid3D(dim, { 2 * width, height, spacingZ });
        Float3 groupOffset = end3_1 + Float3(-width*5.f, +height / 2, gimmick3_0);

        for (const auto& pos : gridPattern) {
             
            auto actor = Mesh::CreateBoxActor(Float3{ 1.0f, 1.0f, 1.0f });

			Mesh::SetBox(actor.get(), refShape);
             
			Float3 targetPos = pos + groupOffset;

			actor->RootComponent->SetRelativePosition(targetPos);
            actor->RootComponent->UpdateWorldTransform();

            auto& rb = actor->shapeComponent->rigidBody;
            rb->simulatePhysics = true;
            rb->simulateRotation = true;
            //actor->staticMeshComponent->SetMaterial(Materials::GetIron()); 

			rb->SetPhysicalMaterial(PhysicalMaterial{ 0.0f, 0.3f });
            rb->linearDamping = 0.99f;
            rb->angularDamping = 0.98f;

            rb->compliance = 0.0001f;

            rb->bFastStable = true;

            this->AddActor(actor);
        }
    }
 



	//============================
    {
        Float3 goalPos = end3_2 + Float3(0.0f, 0.0f, -0.2f);
        Float3 refShape = { LANE_WIDTH, 10.0f, 1.0f };

        auto checkPt = CreateShared<ATriggerVolume>();

        Mesh::SetBox(checkPt.get(), refShape);

        checkPt->RootComponent->SetRelativePosition(goalPos);
        checkPt->RootComponent->UpdateWorldTransform();


        this->AddActor(checkPt);
    }

	//this->playerSpawnPos = end2_2 + Float3(0.0f, 2.0f, -2.0f);

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