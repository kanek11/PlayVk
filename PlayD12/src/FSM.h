#pragma once
#include "PCH.h"
#include "Base.h"

#include <vector>
#include <memory>
#include <functional>
#include <string>
	
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

	virtual void OnStateEnter() {};
	virtual void OnStateExit() {};
	virtual void OnStateUpdate(float dt) {}; 
};


enum class GameStateId
{
	MainMenu,
	Loading,
	Playing,
	Paused,
	GameOver,
};


class GameState : public IState {
public:
	virtual ~GameState() = default;
	virtual GameStateId GetStateId() const = 0;
};


template<GameStateId typeID>
class GameStateImpl : public GameState {
public:
	static constexpr GameStateId Id = typeID;

	GameStateId GetStateId() const override { return Id; } 
};



class GameStateManager : public IFSMController<GameStateId, GameState> {
public:
	template <typename TState>
	void Register() {
		static_assert(std::is_base_of_v<GameState, TState>, "Must derive from GameState");
		constexpr GameStateId id = TState::Id;
		states[id] = CreateShared<TState>();
	}

	void TransitState(GameStateId newState) override {
		if (newState == current) return;
		if (states.contains(current)) states[current]->OnStateExit();
		current = newState;
		if (states.contains(current)) states[current]->OnStateEnter();
	}

	void Update(float dt) override {
		if (states.contains(current)) states[current]->OnStateUpdate(dt);
	}

	GameStateId GetCurrentState() const override {
		return current;
	}

private:
	std::unordered_map<GameStateId, SharedPtr<GameState>> states;
	GameStateId current = GameStateId::MainMenu;
};