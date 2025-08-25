#include "PCH.h"
#include "GameUI.h"

#include "Gameplay/World.h"
#include "GameState.h"

#include "Application.h"

#include "UI.h"

#include "FSM.h"


using namespace UI;

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

void UGameplayUI::Tick(float delta)
{
    UIElement::Tick(delta);

    if (!bRevInput) return;

    auto inputSystem = GameApplication::GetInstance()->GetInputSystem();
    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    //very simple navigation/ focus system
    if (buttons.empty()) {
        return;
    }
    buttonCount = static_cast<int>(buttons.size());
    ////std::cout << "axis: " << axis << '\n';
    if (inputSystem->GetAction(EAction::NavigateUp)) {
        focusIndex = (focusIndex - 1 + buttonCount) % buttonCount;
        uiManager->SetFocus(buttons[focusIndex]);
    }
    else if (inputSystem->GetAction(EAction::NavigateDown)) {
        focusIndex = (focusIndex + 1) % buttonCount;
        uiManager->SetFocus(buttons[focusIndex]);
    } 

    if (inputSystem->GetAction(EAction::Confirm)) {
        if (focusIndex < buttonCount) {
            buttons[focusIndex]->InvokeClick(true);
            std::cout << "confirm index:" << focusIndex << '\n';
        }
    }

}

void UGameplayUI::OnRegister()
{
    UIElement::OnRegister();

    this->bRevInput = true;

    auto uiManager = GameApplication::GetInstance()->GetUIManager();

    buttonCount = static_cast<int>(buttons.size());
    if (focusIndex < buttonCount) {
        uiManager->SetFocus(buttons[focusIndex]);
        std::cout << "\tdefault index:" << focusIndex << '\n';
        //bDefaultFocus = false; //only focus once
    }
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
        speedHUD->text = std::format("{:.1f} m/s", currSpeed);

        //accelHUD->text = std::format("acc:{:.1f}", std::clamp(playerState.accel,0.0f, 200.0f));
    }


    auto& ability = playerState.abilitiesRT;

    int index{ 0 };
    for (auto& hud : abilityRTs) {

        EPlayerForm form = static_cast<EPlayerForm>(index + 1);
        if(ability.contains(form))
        hud->text = std::format("{:.1f}", ability.at(form).remaining);
        index++;
    }


}

void UPlayerHUD::LateConstruct()
{
    auto uiManager = GameApplication::GetInstance()->GetUIManager();
    auto aspectRatio = GameApplication::GetInstance()->getAspectRatio();

    float speedH = 0.1f;
    {
        //debugHUD1 = uiManager->CreateUIAsRoot<UIButton>();
        speedHUD = canvas->CreateUIAsChild<UITextBlock>();

        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(speedH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(1.0f - speedH);

        style.policy = SizePolicy::AutoText;
        style.alignment = Alignment::Center;

        speedHUD->text = std::format("vel:{:.1f}", 0.0f);
        speedHUD->SetLayoutStyle(style);

        speedHUD->name = "PlayerSpeed";
    }


    float iconW = 0.1f / aspectRatio;
    float abilityH = 0.1f;  

    std::array<std::string, 3> textures = {
        "metal.tif",
        "crystal.tif",
        "break.tif",
    };

    {
        int index{ 0 };
        for (auto& hud : abilityIcon) {

            hud = canvas->CreateUIAsChild<UITextBlock>();

            LayoutStyle style{};
            style.width = UISize::Pc(iconW);
            style.height = UISize::Pc(abilityH);
            style.offsetX = UISize::Pc(0.0f);
            style.offsetY = UISize::Pc(abilityH * index);

            style.policy = SizePolicy::Fixed;
            style.alignment = Alignment::Left;

            hud->SetLayoutStyle(style);


            hud->backAlpha = { 1.0f,1.0f,1.0f,1.0f };
            hud->text = "";
            hud->backTex = textures[index];

            index++;
        }
    }


    {
        int index{ 0 };
        for (auto& hud : abilityRTs) {

            hud = canvas->CreateUIAsChild<UITextBlock>();

            LayoutStyle style{};
            style.width = UISize::Pc(1.0f);
            style.height = UISize::Pc(abilityH);
            style.offsetX = UISize::Pc(iconW);
            style.offsetY = UISize::Pc(abilityH * index);

            style.policy = SizePolicy::AutoText;
            style.alignment = Alignment::Left;

            hud->text = "";
            hud->SetLayoutStyle(style);
            index++;
        }  
         
    }






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

    float titleH = 0.3f;
    float buttonH = 0.1f;
    float spacingH = 0.03f;

    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(titleH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(0);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        titleHUD = canvas->CreateUIAsChild<UITextBlock>();  //CreateShared<UITextBlock>(); 
        titleHUD->text = "Title";
        titleHUD->SetLayoutStyle(style);
        //titleHUD->AttachTo(canvas.get()); 
    }


    {

        startButton = canvas->CreateUIAsChild<UIButton>();

        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(titleH + spacingH);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        startButton->text = "Enter";
        startButton->SetLayoutStyle(style);

        startButton->OnClick.Add([=]() {
            std::cout << "click start" << '\n';
            float duration = 1.0f;
            gameState->RequestTransitGameState(GameStateId::Playing, duration);
            gameState->CameraToPlay(duration);
            this->bRevInput = false;
            });
    }


    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(titleH + buttonH + spacingH + spacingH);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        quitButton = canvas->CreateUIAsChild<UIButton>();

        quitButton->text = "Quit";
        quitButton->SetLayoutStyle(style);

        quitButton->OnClick.Add([=]() {
            std::cout << "click quit" << '\n';
            GameApplication::GetInstance()->GetMainWindow()->RequestClose();
            });

    }

    this->buttons.push_back(startButton.get());
    this->buttons.push_back(quitButton.get());
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

    float startY = 0.2f;
    float buttonH = 0.1f;
    float spacingH = 0.05f;
    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        resumeButton = canvas->CreateUIAsChild<UIButton>();
        resumeButton->text = "Resume";

        resumeButton->SetLayoutStyle(style);

        resumeButton->OnClick.Add([=]() {
            gameState->RequestTransitGameState(GameStateId::Playing);
            });
    }

    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY + spacingH + buttonH);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        retryButton = canvas->CreateUIAsChild<UIButton>();

        retryButton->text = "retry";
        retryButton->SetLayoutStyle(style);


        retryButton->OnClick.Add([=]() {
            gameState->OnResetGameplay();

            float duration = 0.1f;
            gameState->RequestTransitGameState(GameStateId::Playing);
            });
    }

    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY + spacingH + buttonH + spacingH + buttonH);

        style.policy = SizePolicy::AutoText;
        //style.alignment = Alignment::Center;

        returnButton = canvas->CreateUIAsChild<UIButton>();

        returnButton->text = "title";
        returnButton->SetLayoutStyle(style);


        returnButton->OnClick.Add([=]() {
            gameState->RequestTransitGameState(GameStateId::MainTitle);
            });
    }

    this->buttons.push_back(resumeButton.get());
    this->buttons.push_back(retryButton.get());
    this->buttons.push_back(returnButton.get());
}


//---------------------------
UGoalingUI::UGoalingUI() : UGameplayUI()
{

}

void UGoalingUI::Tick(float delta)
{
    UGameplayUI::Tick(delta);

    auto gameState = GetWorld()->GetGameState<AGameState>();
    auto& playerState = gameState->PullPlayerState();

    if (gameState->newRecord) {
        newRecordButton->text = "new record!";
    }
    else {
        newRecordButton->text = "";
    }

}

void UGoalingUI::LateConstruct()
{

    auto gameState = GetWorld()->GetGameState<AGameState>();

    float startY = 0.3f;
    float buttonH = 0.1f;
    float spacingH = 0.05f;
    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH * 0.5f);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(0.2f);

        newRecordButton = canvas->CreateUIAsChild<UITextBlock>();
        newRecordButton->text = "";
        newRecordButton->SetLayoutStyle(style);

        newRecordButton->baseColor = Color::Yellow.xyz();
    }


    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY);

        recordButton = canvas->CreateUIAsChild<UITextBlock>();

        recordButton->text = std::format("Record:{:.2f}", gameState->timeCount);
        recordButton->SetLayoutStyle(style);

    }

    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY + spacingH + buttonH);

        retryButton = canvas->CreateUIAsChild<UIButton>();

        retryButton->text = "retry";
        retryButton->SetLayoutStyle(style);

        retryButton->OnClick.Add([=]() {
            gameState->OnResetGameplay();
            gameState->RequestTransitGameState(GameStateId::Playing); 
            });
    }

    {
        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(buttonH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(startY + spacingH + buttonH + spacingH + buttonH);

        returnButton = canvas->CreateUIAsChild<UIButton>();
        returnButton->text = "title";

        returnButton->SetLayoutStyle(style);

        returnButton->OnClick.Add([=]() {
            gameState->RequestTransitGameState(GameStateId::MainTitle);
            });
    }

    this->buttons.push_back(retryButton.get());
    this->buttons.push_back(returnButton.get());
}

void UGoalingUI::OnRegister()
{
    auto gameState = GetWorld()->GetGameState<AGameState>();

    recordButton->text = std::format("Record:{:.2f}", gameState->timeCount);
}


//------------
UGameStatsHUD::UGameStatsHUD() :UGameplayUI()
{
}

void UGameStatsHUD::Tick(float delta)
{
    UGameplayUI::Tick(delta);
    auto gameState = GetWorld()->GetGameState<AGameState>();
    {
        timeHUD->text = std::format("{:.2f}", gameState->timeCount);

        if (!gameState->startPlaying && gameState->countDown >= 0) {

            //std::cout << "count down: " << gameState->countDown << '\n';
            countDown->text = std::format("{:d}", static_cast<int>(std::ceil(gameState->countDown)));
        }
        else
        {
            countDown->text = "";
        }
    }

}

void UGameStatsHUD::LateConstruct()
{

    float timeH = 0.05f; 
    {
        timeHUD = canvas->CreateUIAsChild<UITextBlock>();

        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(timeH);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(1 - timeH);

        style.policy = SizePolicy::AutoText;
        style.alignment = Alignment::Left;

        timeHUD->name = "Current Time";
        timeHUD->text = std::format("{:.1f}m/s", 0.0f);
        timeHUD->SetLayoutStyle(style);
    }

    float countDownH = 0.2f;
    {
        countDown = canvas->CreateUIAsChild<UITextBlock>();

        LayoutStyle style{};
        style.width = UISize::Pc(1.0f);
        style.height = UISize::Pc(0.2f);
        style.offsetX = UISize::Pc(0);
        style.offsetY = UISize::Pc(0.3f);

        style.policy = SizePolicy::AutoText;
        style.alignment = Alignment::Center;

        countDown->name = "count down";
        countDown->text = ""; //std::format("{:d}", 0);
        countDown->SetLayoutStyle(style);
    }


}