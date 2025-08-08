#pragma once

#include "Base.h"
#include "Gameplay/Actor.h"

#include "UI.h"

using namespace Gameplay;


class AUIManager : public AActor {
public:
	//AUIManager();
	virtual void BeginPlay();
	virtual void OnTick(float delta);

protected:
	std::vector<SharedPtr<UIButton>> m_buttons;
};

class APlayerHUD : public AUIManager {
public:
   APlayerHUD(); 

   virtual void OnTick(float delta);

   SharedPtr<UIButton> debugHUD1; 
   SharedPtr<UIButton> debugHUD2; 
   SharedPtr<UIButton> debugHUD3;
};
