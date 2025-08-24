#pragma once

#include "Base.h"
#include "Gameplay/Actor.h"

#include "Gameplay/World.h"
#include "Gameplay/Actors/AController.h"	

#include "UI.h"

/*
* design decision:
* don't bypass factory to construct gameplay ui;
*
* usually we need a valid world for UI;
* so we impose an "late construction" hook;
*
* realize constructor shouldn't do "behavior stuff" , only memory construction;

*/

using namespace Gameplay;


class UGameplayUI : public UIElement {
public:
	UGameplayUI(); 

	UWorld* GetWorld() const {
		assert(world != nullptr);
		return world;
	}
	void SetWorld(UWorld* world) { this->world = world; }

	//for valid world;
	virtual void LateConstruct() = 0;

	virtual void Tick(float delta);

private:
	UWorld* world{ nullptr };

public:
	//as feature, a full-screen, default canvas is given; but could be bypassed, resized, etc.
	SharedPtr<UICanvasPanel> canvas;

public:
	std::vector<UIButton*> buttons;
	int buttonCount{ 0 };
	int focusIndex{ 0 };

	bool bDefaultFocus{ true }; 
};


template <DerivedFrom<UGameplayUI> T>
SharedPtr<T> CreateGameplayUI(UWorld* world)
{
	SharedPtr<T> ui = CreateShared<T>();
	ui->SetWorld(world);
	ui->LateConstruct();
	return ui;
}


class UGameStatsHUD : public UGameplayUI
{
public:
	UGameStatsHUD();
	virtual void Tick(float delta) override;
	virtual void LateConstruct() override;

	SharedPtr<UITextBlock> timeHUD;

	SharedPtr<UITextBlock> countDown;
};



class UMainTitleUI : public UGameplayUI
{
public:
	UMainTitleUI();
	virtual void Tick(float delta) override;
	virtual void LateConstruct() override;

	SharedPtr<UITextBlock> titleHUD;
	SharedPtr<UIButton> startButton;
	SharedPtr<UIButton> quitButton;
};

class UPlayerHUD :public UGameplayUI
{
public:
	UPlayerHUD();

	virtual void Tick(float delta) override;
	virtual void LateConstruct() override;

	SharedPtr<UITextBlock> speedHUD;

	std::array<SharedPtr<UITextBlock>, 3> abilityRT;

	std::array<SharedPtr<UITextBlock>, 3> abilityIcon;
};


class UPauseMenu : public UGameplayUI
{
public:
	UPauseMenu();
	virtual void Tick(float delta) override;
	virtual void LateConstruct() override;

	SharedPtr<UIButton> resumeButton;
	SharedPtr<UIButton> retryButton;
	SharedPtr<UIButton> returnButton;
};


class UGoalingUI : public UGameplayUI
{
public:
	UGoalingUI();
	virtual void Tick(float delta) override;
	virtual void LateConstruct() override;

	virtual void OnRegister() override;

	SharedPtr<UITextBlock> recordButton;
	SharedPtr<UIButton> retryButton;
	SharedPtr<UIButton> returnButton;

	SharedPtr<UITextBlock> newRecordButton;
};
