#include "PCH.h"
#include "GameState.h"

#include "Application.h"

AGameState::AGameState() : AGameStateBase()
{
	this->onGoal.Add([=]() {
		this->OnGoalReached();
		});

}

void AGameState::OnTick(float delta)
{
	m_gameManager->Update(delta);

}

void AGameState::BeginPlay()
{ 
	InitUI();

	//after world is ready
	SetupGameStates();
}

void AGameState::SetupGameStates()
{
	auto world = this->GetWorld();  
	auto pc = world->GetFirstPlayerController();

	auto uiManager = GameApplication::GetInstance()->GetUIManager(); 
	auto inputSystem = GameApplication::GetInstance()->GetInputSystem();
	auto timeSystem = GameApplication::GetInstance()->GetTimeSystem();

    //
	{
		auto mainMenuState = m_gameManager->Register<MainTitleState>();

		mainMenuState->OnEnter.Add(
			[=]() {
				std::cout << "state: enter mainMenu " << "\n";
				uiManager->RegisterRootElement(mainTitle.get());

				this->OnResetGameplay();
				pc->SetInputMode(EInputMode::UIOnly);
			});

		mainMenuState->OnUpdate.Add(
			[=](float dt) {
				//std::cout << " tick mainMenu: " << dt << "\n";
			});
		mainMenuState->OnExit.Add(
			[=]() {
				uiManager->UnregisterRootElement(mainTitle.get());
			});

		//m_gameManager->SetInitialState(mainMenuState->GetId());

	}



	{
		auto playingState = m_gameManager->Register<PlayingState>();

		playingState->OnEnter.Add(
			[=]() {
				std::cout << "state: enter playing" << "\n";
				//world->TransitLevel("gameplay");
				//world->LoadOrResetLevel("gameplay");
				pc->SetInputMode(EInputMode::None);
				uiManager->RegisterRootElement(playerHUD.get());
			});

		playingState->OnUpdate.Add(
			[=](float dt) {
				this->timeCount += dt;

				//std::cout << " tick playing: " << dt << "\n";
				if (inputSystem->IsKeyJustPressed(KeyCode::Escape)) {
					this->RequestTransitState(GameStateId::Paused);
				}

				if (inputSystem->IsKeyJustPressed(KeyCode::P)) {
					this->OnResetGameplay();
				}

			});

		playingState->OnExit.Add(
			[=]() {
				std::cout << "state: exit playing" << "\n";
				uiManager->UnregisterRootElement(playerHUD.get());
			});
	}
	


	//---------------------
	{
		auto pausedState = m_gameManager->Register<PausedState>();
		pausedState->OnEnter.Add(
			[=]() {
				std::cout << "enter pause state" << "\n";
				uiManager->RegisterRootElement(pauseMenu.get());
				timeSystem->SetPaused(true);
			});

		pausedState->OnUpdate.Add(
			[=](float dt) {
				//std::cout << " tick pause: " << dt << "\n";
				if (inputSystem->IsKeyJustPressed(KeyCode::Escape)) {
					this->RequestTransitState(GameStateId::Playing);
				}

			});

		pausedState->OnExit.Add(
			[=]() {
				std::cout << " exit pause state" << "\n";
				uiManager->UnregisterRootElement(pauseMenu.get());
				timeSystem->SetPaused(false);
			});

	}




 
	//----------------------------  

	//m_gameManager->SetInitialState(playingState->GetId());
	m_gameManager->SetInitialState(MainTitleState::GetId());
	m_gameManager->Initialize();

}



void AGameState::InitUI()
{ 
	//auto uiManager = GameApplication::GetInstance()->GetUIManager();

	this->playerHUD = CreateGameplayUI<UPlayerHUD>(this->GetWorld());   
	this->playerHUD->name = "PlayerHUD";

	this->mainTitle = CreateGameplayUI<UMainTitleUI>(this->GetWorld());
	this->mainTitle->name = "MainTitleUI";

	this->pauseMenu = CreateGameplayUI<UPauseMenu>(this->GetWorld());
	this->pauseMenu->name = "PauseMenuUI";

	//uiManager->RegisterRootElement(playerHUD.get());
	//uiManager->UnregisterRootElement(playerHUD.get());
}



void AGameState::OnResetGameplay()
{ 
	std::cout << "gamestate: reset game!" << std::endl;

	auto world = this->GetWorld();
	world->LoadOrResetLevel("gameplay");

	this->timeCount = 0.0f;
}


void AGameState::OnGoalReached()
{
	std::cout << "Goal reached!" << std::endl;
	this->RequestTransitState(GameStateId::Goaling);
}