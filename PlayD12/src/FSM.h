#pragma once
#include "PCH.h"
#include "Base.h"

#include "Delegate.h"
  

template<typename TStateEnum, typename TStateObject>
class IFSMController {
public:
	virtual ~IFSMController() = default;
	virtual void Update(float delta) = 0;
	virtual void TransitState(TStateEnum newState) = 0;
	virtual TStateEnum GetCurrentState() const = 0;
};

class IState {
public:
	virtual ~IState() = default;

	virtual void OnStateEnter() {
		OnEnter.BlockingBroadCast();
	};
	virtual void OnStateExit() {
		OnExit.BlockingBroadCast();
	};
	virtual void OnStateUpdate(float dt) {
		OnUpdate.BlockingBroadCast(dt);
	};

	FDelegate<void()> OnEnter;
	FDelegate<void()> OnExit;
	FDelegate<void(float)> OnUpdate;
};


enum class GameStateId
{
	MainMenu,
	Loading,
	Playing,
	Paused,
	Goaling,
	GameOver,
};


class GameState : public IState {
public:
	virtual ~GameState() = default; 
};


template<GameStateId typeID>
class GameStateImpl : public GameState {
public:
	static constexpr GameStateId Id = typeID; 
	static constexpr GameStateId GetId() { return Id; }
};



class GameStateManager : public IFSMController<GameStateId, GameState> {
public:
	template <typename State> [[nodiscard]]
	SharedPtr<State> Register() {
		static_assert(std::is_base_of_v<GameState, State>, "Must derive from GameStateImpl");
		constexpr GameStateId id = State::Id;
		auto statePtr = CreateShared<State>();
		states[id] = statePtr;
		return statePtr;
	}

	void RequestTransitState(GameStateId newState) {
		if (newState == current) return; 
		target = newState;
	}


	void TransitState(GameStateId newState) override {
		if (newState == current) return;
		assert(states.contains(current) && "Initial state must be registered before calling Initialize()");
		if (states.contains(current)) states[current]->OnStateExit();
		current = newState;
		if (states.contains(current)) states[current]->OnStateEnter();
	}

	void SetInitialState(GameStateId state) {
		current = state;
		target = state;
	}

	void Initialize() { 
		assert(states.contains(current) && "Initial state must be registered before calling Initialize()");
		if (states.contains(current)) {
			states[current]->OnStateEnter();
		}
	}

	void Update(float dt) override {
		if (target != current) this->TransitState(target);
		if (states.contains(current)) states[current]->OnStateUpdate(dt);
	}

	[[nodiscard]]
	GameStateId GetCurrentState() const override {
		return current;
	}

private:
	//or just array;
	std::unordered_map<GameStateId, SharedPtr<GameState>> states;
	GameStateId current = GameStateId::MainMenu;

	GameStateId target = GameStateId::MainMenu;  
};


class MainMenuState : public GameStateImpl<GameStateId::MainMenu> {
	 
};

class PlayingState : public GameStateImpl<GameStateId::Playing> {

};

class PausedState : public GameStateImpl<GameStateId::Paused> {
};

class GoalingState : public GameStateImpl<GameStateId::Goaling> {
};

