#pragma once

#include "Base.h"
#include "Gameplay/Actor.h"

#include "Gameplay/World.h"
#include "Gameplay/Actors/AController.h"	

#include "UI.h"

using namespace Gameplay;

 
class UGamePlayUI : public UIElement {
public:
	UWorld* GetWorld() const { return world; }
	void SetWorld(UWorld* world) { this->world = world; }
	 
	virtual void Tick(float delta) {}; 

private:
	UWorld* world{ nullptr }; 
};

 
template <DerivedFrom<UGamePlayUI> T>
SharedPtr<T> CreateGameplayUI(UWorld* world) 
{
	SharedPtr<T> ui = CreateShared<T>();
	ui->SetWorld(world);
	return ui;
}


class UGameStatsUI : public UGamePlayUI
{ 
public:
	UGameStatsUI();
	virtual void Tick(float delta) override;
	SharedPtr<UICanvasPanel> canvas; 
};
 
class UMainTitle : public UGamePlayUI
{
public:
	UMainTitle();
	virtual void Tick(float delta) override;
	SharedPtr<UICanvasPanel> canvas;
	SharedPtr<UIButton> startButton;
	SharedPtr<UIButton> quitButton;
};

class UPlayerHUD :public UGamePlayUI
{
public:
	UPlayerHUD();

	virtual void Tick(float delta) override;

	SharedPtr<UICanvasPanel> canvas;
	SharedPtr<UIButton> debugHUD1;
	SharedPtr<UIButton> debugHUD2;
	SharedPtr<UIButton> debugHUD3;
};


class UPauseMenu : public UGamePlayUI
{
public:
	UPauseMenu();
	virtual void Tick(float delta) override;
	SharedPtr<UICanvasPanel> canvas;
	SharedPtr<UIButton> resumeButton;
	SharedPtr<UIButton> quitButton;
};


class UGoalingUI : public UGamePlayUI
{ 
public:
	UGoalingUI();
	virtual void Tick(float delta) override;
	SharedPtr<UICanvasPanel> canvas;
	SharedPtr<UIButton> goalButton;
	SharedPtr<UIButton> quitButton;
	SharedPtr<UIButton> restartButton;
};

 