#include "PCH.h"
#include "GameState.h"

#include "Application.h"

AGameState::AGameState(): AGameStateBase()
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

	SetupUI();

	//after world is ready
	SetupGameStates(); 
}

void AGameState::SetupGameStates()
{
	auto world = this->GetWorld();

	auto mainMenuState = m_gameManager->Register<MainMenuState>();
	 
	//
	mainMenuState->OnEnter.Add(
		[&]() {
			std::cout << " enter mainMenu state" << "\n";
			world->TransitLevel("mainMenu");
			//mainMenuWorld->OnLoad();
		});

	mainMenuState->OnUpdate.Add(
		[&](float dt) {
			//mainMenuWorld->OnUpdate(dt);
			//std::cout << " tick mainMenu: " << dt << "\n";
		});
	mainMenuState->OnExit.Add(
		[&]() {
			//mainMenuWorld->OnUnload();
			//std::cout << " tick mainMenu: " << dt << "\n";
		});

		//m_gameManager->SetInitialState(mainMenuState->GetId());
		 
 
	 
	//
	auto playingState = m_gameManager->Register<PlayingState>();

	playingState->OnEnter.Add(
		[&]() {
			std::cout << " enter playing state" << "\n";
			//gameWorld->OnLoad();

			world->TransitLevel("gameplay");
		});

	playingState->OnUpdate.Add(
		[&](float dt) {
			//gameWorld->OnUpdate(dt);
			//std::cout << " tick playing: " << dt << "\n";
		});

	playingState->OnExit.Add(
		[&]() {
			std::cout << " exit playing state" << "\n";
			//gameWorld->OnUnload();
		});


	//---------------------
	auto pausedState = m_gameManager->Register<PausedState>();
	pausedState->OnEnter.Add(
		[&]() {
			std::cout << " enter pause state" << "\n";
			//pauseWorld->OnLoad();
		});

	pausedState->OnUpdate.Add(
		[&](float dt) {
			//pauseWorld->OnUpdate(dt);
			//std::cout << " tick pause: " << dt << "\n";
		});

	pausedState->OnExit.Add(
		[&]() {
			std::cout << " exit pause state" << "\n";
			//pauseWorld->OnUnload();
		});

	//----------------------------  

	m_gameManager->SetInitialState(playingState->GetId());
	m_gameManager->Initialize();

}



void AGameState::SetupUI()
{
 
	auto uiManager = GameApplication::GetInstance()->GetUIManager();

	this->playerHUD = CreateGameplayUI<UPlayerHUD>(this->GetWorld());

	uiManager->RegisterRootElement(this->playerHUD.get());
	 
}

void AGameState::OnGoalReached()
{
	std::cout << "Goal reached!" << std::endl;
}
