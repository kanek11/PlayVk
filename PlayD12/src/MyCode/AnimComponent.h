#pragma once



#include "SDL/SDL.h"
#include "../Component.h"
 
#include "FSMController.h"	

class FAnimComponent : public Component {
public:
	FAnimComponent(class Actor* owner, int drawOrder = 100);
	~FAnimComponent() = default;

	void Update(float dt) override;


	std::shared_ptr<FSMController> controller;
	std::function<void(SDL_Texture*)>  setTexture_fn;
	std::function<bool()> jumpCond;
};


