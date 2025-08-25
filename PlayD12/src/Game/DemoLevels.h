#pragma once

#include "Gameplay/Level.h" 
 
constexpr float LANE_WIDTH = 20.0f;  

using namespace Gameplay;

class GlobalLevel : public ULevel {
public:
	virtual ~GlobalLevel() = default;
	virtual void OnLoad() override;
	virtual void OnUnload() override;
	virtual void OnTick(float delta) override;
};

class GamePlayLevel : public ULevel {
public:
	virtual ~GamePlayLevel() = default;
	virtual void OnLoad() override;
	virtual void OnUnload() override;
	virtual void OnTick(float delta) override;

	void LoadPlayer(); 
	void LoadMap0();

public: 
	Float3 playerSpawnPos = { 0.0f, 1.1f, 2.0f };
};
