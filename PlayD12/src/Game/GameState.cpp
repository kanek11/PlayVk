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
				//todo: camera is not ready
				//auto arm = this->GetWorld()->GetActiveCameraArm(); 
				//arm->LocalOffset = titleCamOff; 

				mainTitle->SetOpacityHierarchy(1.0f);
				//
				uiManager->RegisterRootElement(mainTitle.get());


				this->OnResetGameplay();

				//debug: enter game automatically; after scene loaded
				//mainTitle->startButton->OnClick.BlockingBroadCast();

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

	}

	{
		auto playingState = m_gameManager->Register<PlayingState>();

		playingState->OnEnter.Add(
			[=]() {
				std::cout << "state: enter playing" << "\n";
				//world->TransitLevel("gameplay");
				//world->LoadOrResetLevel("gameplay");
				//this->OnResetGameplay();
				if (lastState != GameStateId::Paused)
					this->OnStartPlay(1.0f);
				else {
					this->OnStartPlay(0.0f);
				}

				uiManager->RegisterRootElement(playerHUD.get());
				uiManager->RegisterRootElement(gameStats.get());


			});

		playingState->OnUpdate.Add(
			[=](float dt) {
				if (!startPlaying) return;

				this->timeCount += dt;

				if (shouldRespawn) {

					this->OnResetGameplay();
					this->RequestTransitGameState(GameStateId::Playing); 
					shouldRespawn = false;
				}

				//std::cout << " tick playing: " << dt << "\n";
				if (inputSystem->GetAction(EAction::Pause)) {
					this->RequestTransitGameState(GameStateId::Paused);
					/*timeSystem->TogglePaused();*/

					paused = true;
				}


				//if (inputSystem->IsKeyJustPressed(KeyCode::P)) {
				//	this->OnResetGameplay();
				//}

			});

		playingState->OnExit.Add(
			[=]() {
				std::cout << "state: exit playing" << "\n";
				uiManager->UnregisterRootElement(playerHUD.get());
				uiManager->UnregisterRootElement(gameStats.get());
				//
				startPlaying = false;
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

				pauseMenu->canvas->baseColor = Color::Black.xyz();
				pauseMenu->canvas->backAlpha = { 0.2f, 0.2f, 0.2f, 0.2f };
			});

		pausedState->OnUpdate.Add(
			[=](float dt) {
				//std::cout << " tick pause: " << dt << "\n";
				if (inputSystem->GetAction(EAction::Pause)) {
					this->RequestTransitGameState(GameStateId::Playing);
				}

				if (inputSystem->IsKeyJustPressed(KeyCode::L)) {
					timeSystem->AdvanceFixedSteps();
					timeSystem->AdvanceFrames();
				}

			});

		pausedState->OnExit.Add(
			[=]() {
				std::cout << " exit pause state" << "\n";
				uiManager->UnregisterRootElement(pauseMenu.get());
				timeSystem->SetPaused(false);
			});
	}


	{

		auto goalingState = m_gameManager->Register<GoalingState>();
		goalingState->OnEnter.Add(
			[=]() {
				std::cout << "enter pause state" << "\n";
				uiManager->RegisterRootElement(goalUI.get());
				pc->SetInputMode(EInputMode::UIOnly);

				if (this->timeCount < bestRecord) {
					bestRecord = timeCount;
					newRecord = true;
				}

			});

		goalingState->OnUpdate.Add(
			[=](float dt) {

			});

		goalingState->OnExit.Add(
			[=]() {
				std::cout << " exit pause state" << "\n";
				uiManager->UnregisterRootElement(goalUI.get());

				newRecord = false;
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

	this->goalUI = CreateGameplayUI<UGoalingUI>(this->GetWorld());
	this->goalUI->name = "GoalUI";

	this->gameStats = CreateGameplayUI<UGameStatsHUD>(this->GetWorld());
	this->gameStats->name = "GameStatsHUD";

	//uiManager->RegisterRootElement(playerHUD.get());
	//uiManager->UnregisterRootElement(playerHUD.get());
}



void AGameState::OnResetGameplay()
{
	std::cout << "gamestate: reset game!" << std::endl;

	auto world = this->GetWorld();
	world->RequestReloadLevel("gameplay");

}


void AGameState::OnGoalReached()
{
	std::cout << "Goal reached!" << std::endl;
	this->RequestTransitGameState(GameStateId::Goaling);
}