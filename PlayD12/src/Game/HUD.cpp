#include "PCH.h"
#include "HUD.h"

#include "Gameplay/World.h"
#include "GameState.h"

#include "Application.h"
 
APlayerHUD::APlayerHUD()
{
    FRect buttonRect = { 10, 20, 300, 50 };
    debugHUD1 = CreateShared<UIButton>(buttonRect);

    FRect buttonRect1 = { 10, 100, 300, 50 };
    debugHUD2 = CreateShared<UIButton>(buttonRect1);

    FRect buttonRect2 = { 10, 200, 300, 50 };
    debugHUD3 = CreateShared<UIButton>(buttonRect2);

    m_buttons.push_back(debugHUD1);
    m_buttons.push_back(debugHUD2);
    m_buttons.push_back(debugHUD3); 

    //
}

void AUIManager::BeginPlay()
{
    AActor::BeginPlay();

    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    for (auto& ui : m_buttons) {
        uiManager->RegisterRootElement(ui.get());
    } 
}

void AUIManager::OnTick(float delta)
{
    AActor::OnTick(delta);

    for (auto& ui : m_buttons) {
        ui->Tick(delta);
    }
}




void APlayerHUD::OnTick(float delta)
{
    AUIManager::OnTick(delta);


    auto gameState = GetWorld()->GetGameState<AGameState>(); 
    auto& playerState = gameState->PullPlayerState(); 

    float currSpeed = playerState.speed;
    debugHUD1->text = std::format("vel:{:.2f}", currSpeed);

    //float currDist = goalLength / 2 - debugPlayer->position.z();
    //debugHUD2->text = std::format("dist:{:.2f}", currDist);

    //timeCount += delta;
    //debugHUD3->text = std::format("time:{:.2f}", timeCount);

    //if (inputSystem == nullptr) {
    //    std::cerr << "empty input system" << '\n';
    //    return;
    //}

}

