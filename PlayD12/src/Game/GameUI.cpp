#include "PCH.h"
#include "GameUI.h"

#include "Gameplay/World.h"
#include "GameState.h"

#include "Application.h"

#include "UI.h"

#include "FSM.h"

//-------------------------
UGameplayUI::UGameplayUI() : UIElement()
{
    auto width = GameApplication::GetInstance()->GetWidth();
    auto height = GameApplication::GetInstance()->GetHeight();

    canvas = CreateShared<UICanvasPanel>();
    FRect canvasRect = { 0, 0, static_cast<int>(width), static_cast<int>(height) };
    canvas->SetLayout(canvasRect);
    canvas->name = "PlayerHUDCanvas";

    canvas->AttachTo(this);
}



//-------------------------
UPlayerHUD::UPlayerHUD() : UGameplayUI()
{
}

void UPlayerHUD::Tick(float delta)
{
    UGameplayUI::Tick(delta);

    if (!GetWorld()) {
        std::cerr << "UPlayerHUD: World is null!" << std::endl;
        return;
    }
    auto gameState = GetWorld()->GetGameState<AGameState>();
    auto& playerState = gameState->PullPlayerState();

    {
        float currSpeed = playerState.speed;
        speedHUD->text = std::format("vel:{:.1f}", currSpeed);

        accelHUD->text = std::format("acc:{:.1f}", std::clamp(playerState.accel,0.0f, 200.0f));
    }


}

void UPlayerHUD::LateConstruct()
{
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    {
        //debugHUD1 = uiManager->CreateUIAsRoot<UIButton>();
        speedHUD = CreateShared<UIButton>();

        FRect buttonRect = { 10, 20, 300, 50 };
        speedHUD->SetLayout(buttonRect);
        speedHUD->name = "PlayerSpeed";
    }


    {
        //debugHUD1 = uiManager->CreateUIAsRoot<UIButton>();
        accelHUD = CreateShared<UIButton>();

        FRect buttonRect = { 10, 20, 300, 50 };
        buttonRect.y += 60;
        accelHUD->SetLayout(buttonRect);
        accelHUD->name = "PlayerAccel";
    }

 

    //hierarchy
    speedHUD->AttachTo(canvas.get()); 
    accelHUD->AttachTo(canvas.get());
    //FRect buttonRect2 = { 10, 200, 300, 50 };
    //debugHUD3 = CreateShared<UIButton>(buttonRect2);
}


//-------------------------
UMainTitleUI::UMainTitleUI() : UGameplayUI()
{
}

void UMainTitleUI::Tick(float delta)
{
    UGameplayUI::Tick(delta);

}

void UMainTitleUI::LateConstruct()
{
    auto gameState = GetWorld()->GetGameState<AGameState>();

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);

        startButton = CreateShared<UIButton>();
        startButton->SetLayout(centeredRect);
        startButton->text = "Press To Enter";

        startButton->OnClick.Add([=]() {
            std::cout << "click start" << '\n';
            gameState->RequestTransitState(GameStateId::Playing);
            });
    }


    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);
        centeredRect.y += 60;

        quitButton = CreateShared<UIButton>();
        quitButton->SetLayout(centeredRect);
        quitButton->text = "Quit";
    }


    startButton->AttachTo(canvas.get());
    quitButton->AttachTo(canvas.get());
}


//-------------------------
UPauseMenu::UPauseMenu() : UGameplayUI()
{

}

void UPauseMenu::Tick(float delta)
{
    UGameplayUI::Tick(delta);
}

void UPauseMenu::LateConstruct()
{
    auto gameState = GetWorld()->GetGameState<AGameState>();

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);

        resumeButton = CreateShared<UIButton>();
        resumeButton->SetLayout(centeredRect);
        resumeButton->text = "Resume";

        resumeButton->OnClick.Add([=]() {
            gameState->RequestTransitState(GameStateId::Playing);
            });
    }

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);
        centeredRect.y += 60;

        retryButton = CreateShared<UIButton>();
        retryButton->SetLayout(centeredRect);
        retryButton->text = "retry";

        retryButton->OnClick.Add([=]() {
            gameState->OnResetGameplay();
            gameState->RequestTransitState(GameStateId::Playing);
            });
    }

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);
        centeredRect.y += 60 * 2;

        returnButton = CreateShared<UIButton>();
        returnButton->SetLayout(centeredRect);
        returnButton->text = "back to title";

        returnButton->OnClick.Add([=]() {
            gameState->RequestTransitState(GameStateId::MainTitle);
            });
    }

    resumeButton->AttachTo(canvas.get());
    retryButton->AttachTo(canvas.get());
    returnButton->AttachTo(canvas.get());
}


//---------------------------
UGoalingUI::UGoalingUI() : UGameplayUI()
{

}

void UGoalingUI::Tick(float delta)
{
    UGameplayUI::Tick(delta);



}

void UGoalingUI::LateConstruct()
{

    auto gameState = GetWorld()->GetGameState<AGameState>();

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);

        recordButton = CreateShared<UIButton>();
        recordButton->SetLayout(centeredRect);
        recordButton->text = std::format("Record :{:.2f}", gameState->timeCount);

    }

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);
        centeredRect.y += 60;

        retryButton = CreateShared<UIButton>();
        retryButton->SetLayout(centeredRect);
        retryButton->text = "retry";

        retryButton->OnClick.Add([=]() {
            gameState->OnResetGameplay();
            gameState->RequestTransitState(GameStateId::Playing);
            });
    }

    {
        FRect buttonRect = { 0, 0, 500, 50 };
        FRect centeredRect = CenterRect(canvas->GetLayout(), buttonRect);
        centeredRect.y += 60 * 2;

        returnButton = CreateShared<UIButton>();
        returnButton->SetLayout(centeredRect);
        returnButton->text = "back to title";

        returnButton->OnClick.Add([=]() {
            gameState->RequestTransitState(GameStateId::MainTitle);
            });
    }

    recordButton->AttachTo(canvas.get());
    retryButton->AttachTo(canvas.get());
    returnButton->AttachTo(canvas.get());
}

void UGoalingUI::OnRegister()
{

    auto gameState = GetWorld()->GetGameState<AGameState>();

    recordButton->text = std::format("Record :{:.2f}", gameState->timeCount);
}


//------------
UGameStatsHUD::UGameStatsHUD():UGameplayUI()
{
}

void UGameStatsHUD::Tick(float delta)
{
    UGameplayUI::Tick(delta);
    auto gameState = GetWorld()->GetGameState<AGameState>();
    {
        timeHUD->text = std::format("t:{:.2f}", gameState->timeCount);
    }

}

void UGameStatsHUD::LateConstruct()
{

    {
        timeHUD = CreateShared<UIButton>();

        FRect buttonRect = { 10, 20, 300, 50 };
        buttonRect.y = 720 - 50;
        timeHUD->SetLayout(buttonRect);
        timeHUD->name = "Current Time";
    }
     
    timeHUD->AttachTo(canvas.get());


}
