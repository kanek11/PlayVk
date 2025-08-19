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
	void InitUI();

public:
	void OnGoalReached();
	FDelegate<void()> onGoal;

	void OnResetGameplay();

public:
	FPlayerState& PullPlayerState() { return currPlayerState; }
	void SetPlayerState(const FPlayerState& state) { currPlayerState = state; }
	FPlayerState currPlayerState{};

public:
	UniquePtr<GameStateManager> m_gameManager = CreateUnique<GameStateManager>();

	void RequestTransitState(const GameStateId& targetState) {
		m_gameManager->RequestTransitState(targetState);
	}

public:
	SharedPtr<UGameStatsHUD> gameStats;

	SharedPtr<UPlayerHUD> playerHUD;

	SharedPtr<UMainTitleUI> mainTitle;

	SharedPtr<UPauseMenu> pauseMenu;

	SharedPtr<UGoalingUI> goalUI;

public:
	float timeCount{ 0.0f };
	float bestRecord = std::numeric_limits<float>::max();
};