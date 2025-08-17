#include "PCH.h"
#include "GameUI.h"

#include "Gameplay/World.h"
#include "GameState.h"

#include "Application.h"
 
#include "UI.h"
 

UPlayerHUD::UPlayerHUD()
{
	auto width = GameApplication::GetInstance()->GetWidth();
	auto height = GameApplication::GetInstance()->GetHeight();
    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    
	canvas = uiManager->CreateUIAsRoot<UICanvasPanel>();
    FRect canvasRect = { 0, 0, static_cast<int>(width), static_cast<int>(height) };
	canvas->SetLayout(canvasRect);  
	canvas->name = "PlayerHUDCanvas";

	//debugHUD1 = uiManager->CreateUIAsRoot<UIButton>();
	debugHUD1 = CreateShared<UIButton>();

    FRect buttonRect = { 10, 20, 300, 50 };
	debugHUD1->SetLayout(buttonRect); 

	//attach as child to canvas
	debugHUD1->AttachTo(canvas.get());
	debugHUD1->name = "DebugHUD1";

    //debugHUD1 = CreateShared<UIButton>(buttonRect);

    //FRect buttonRect1 = { 10, 100, 300, 50 };
    //debugHUD2 = CreateShared<UIButton>(buttonRect1);

    //FRect buttonRect2 = { 10, 200, 300, 50 };
    //debugHUD3 = CreateShared<UIButton>(buttonRect2);
     
}

void UPlayerHUD::Tick(float delta)
{
    if (!GetWorld()) { 
		std::cerr << "UPlayerHUD: World is null!" << std::endl;
		return;
    }
    auto gameState = GetWorld()->GetGameState<AGameState>(); 
    auto& playerState = gameState->PullPlayerState();
    
	float currSpeed = playerState.speed;
    debugHUD1->text = std::format("vel:{:.2f}", currSpeed);

}

 
