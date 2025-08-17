#pragma once

#include "Delegate.h"

#include "Gameplay/GameStateBase.h"

#include "Player.h"

#include "GameUI.h"


using namespace Gameplay;

class AGameState : public AGameStateBase {
public:
	AGameState();

	virtual void OnTick(float delta) override;

	virtual void BeginPlay() override;

private:
	void SetupGameStates();
	void SetupUI();
	 
public:
	void OnGoalReached();  
	FDelegate<void()> onGoal;

public:
	FPlayerState& PullPlayerState() { return currPlayerState; }
	void SetPlayerState(const FPlayerState& state) { currPlayerState = state; }
	FPlayerState currPlayerState{}; 

public: 
	UniquePtr<GameStateManager> m_gameManager = CreateUnique<GameStateManager>(); 

public:
	SharedPtr<UPlayerHUD> playerHUD;
};