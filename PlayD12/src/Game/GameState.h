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



public:
	SharedPtr<UGameStatsHUD> gameStats;

	SharedPtr<UPlayerHUD> playerHUD;

	SharedPtr<UMainTitleUI> mainTitle;

	SharedPtr<UPauseMenu> pauseMenu;

	SharedPtr<UGoalingUI> goalUI;

public:
	//todo: might need more general predicate/timer?  
	//FDelegate<void()> startGame;
	bool startGame{ false };
	float timeCount{ 0.0f };


	float bestRecord = std::numeric_limits<float>::max();
	bool newRecord = false;


	float countDown{ 0.0f };

public:

	void RequestTransitGameState(const GameStateId& targetState, float desiredDelay = 0.0f) {


		Anim::WaitFor(desiredDelay, [=]() {
			m_gameManager->RequestTransitState(targetState);
			});

		//m_gameManager->RequestTransitState(targetState);

	}

	//public:
		//Float3 titleCamOff = { 0.0f, 2.0f, -5.0f };
	Float3 inGameCamOff = { 0.0f, 4.0f, -5.0f };
	Float3 inGameCamRot = Float3{ MMath::ToRadians(30.0f), 0.0f, 0.0f };

	void PlayTitleToPlay(float duration) {

		auto arm = this->GetWorld()->GetActiveCameraArm();
		auto currOffset = arm->LocalOffset;
		auto currEuler = arm->LocalRotationEuler;

		auto anim = Anim::WaitFor(duration,
			[=]() {
			}
		);
		anim->onApply = [=](float nt) {
			arm->LocalOffset = Anim::Lerp(currOffset, inGameCamOff, nt);
			arm->LocalRotationEuler = Anim::Lerp(currEuler, inGameCamRot, nt);

			mainTitle->SetOpacityHierarchy(1.0f - nt);
			};

	}


	void OnEnterPlay() {



	}

};