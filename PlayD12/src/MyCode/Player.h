#pragma once



#include "../Actor.h"

#include "AnimComponent.h"
#include "../SpriteComponent.h"


class Player : public Actor {
public:
	Player(Game* game);

	void UpdateActor(float deltaTime) override;
	void ProcessKeyboard(const uint8_t* state); 

	FAnimComponent* animComp{ nullptr };

	SpriteComponent* spriteComp{ nullptr };
};



struct FAnimClip {
	std::vector<SDL_Texture*> mAnimTextures;
};

class FAnimState : public FState {
public:

	virtual void OnStateUpdate(float dt) override;
	virtual void OnStateEnter() override;
	virtual void OnStateExit() override;

	float currentFrame{ 0 };
	float FPS{ 24 };
	std::shared_ptr<FAnimClip> animClip;

	std::function<void(SDL_Texture*)>  setTexture_fn;
};


class PlayerController : public FSMController {
public:
	PlayerController(Actor* owner, FAnimComponent* ownerComp);

	virtual void Update(float dt) override;

	 
	Actor* mOwner{ nullptr };
	FAnimComponent* mOwnerComp;
};


 


