#pragma once

#include "Gameplay/GameStateBase.h"

#include "Player.h"

using namespace Gameplay;

class AGameState : public AGameStateBase {
public:


	FPlayerState& PullPlayerState() { return currPlayerState; }
	void SetPlayerState(const FPlayerState& state) { currPlayerState = state; }
	FPlayerState currPlayerState;
};