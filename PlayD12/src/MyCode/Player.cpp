#include "Player.h"

#include "../Game.h"


Player::Player(Game* game): Actor(game)
{

	// Create an animated sprite component
	this->animComp = new FAnimComponent(this); 

	this->spriteComp = new SpriteComponent(this);


	animComp->setTexture_fn = [this](SDL_Texture* tex) {
		this->spriteComp->SetTexture(tex);
		};

	this->animComp->jumpCond = []() {return false;};

	//after set texture callback;
	this->animComp->controller = std::make_shared<PlayerController>(this, animComp);
	 

}

void Player::UpdateActor(float deltaTime)
{

	//SDL_Log("update player actor!"); 
 
}

void Player::ProcessKeyboard(const uint8_t* state)
{
	SDL_Log("player press space：%d", state[SDL_SCANCODE_SPACE]);
	this->animComp->jumpCond = [=]() {
		return state[SDL_SCANCODE_SPACE];
		};
}


void FAnimState::OnStateUpdate(float dt)
{

	//SDL_Log("state update");
	if (animClip->mAnimTextures.size() > 0)
	{
		// Update the current frame based on frame rate
		// and delta time
		currentFrame += FPS * dt;

		// Wrap current frame if needed
		if (currentFrame >= animClip->mAnimTextures.size())
		{
			currentFrame -= animClip->mAnimTextures.size();
		}

		//SDL_Log("set current sprite: %f", currentFrame);
		auto currentTexture = animClip->mAnimTextures[static_cast<int>(currentFrame)];
		if (setTexture_fn)
		{
			setTexture_fn(currentTexture);
			//SDL_Log("anim state: set texture");
		}
		else {
			SDL_Log("anim state: no texture callback");
		}
			
	}
}

void FAnimState::OnStateEnter()
{
	SDL_Log("state enter!");
}

void FAnimState::OnStateExit()
{
	SDL_Log("state exit!");
}


PlayerController::PlayerController(Actor* owner, FAnimComponent* ownerComp):
	mOwner(owner), mOwnerComp(ownerComp)
{
	auto game = owner->GetGame(); 

	//
	auto idleState = std::make_shared<FAnimState>();  
	this->stateTable.push_back(idleState);

	idleState->setTexture_fn = ownerComp->setTexture_fn;
	
	std::vector<SDL_Texture*> anims = {
game->GetTexture("Assets/Character01.png"),
game->GetTexture("Assets/Character02.png"),
game->GetTexture("Assets/Character03.png"),
game->GetTexture("Assets/Character04.png"),
game->GetTexture("Assets/Character05.png"),
game->GetTexture("Assets/Character06.png"),
	};

	idleState->animClip = std::make_shared<FAnimClip>();
	idleState->animClip->mAnimTextures = anims;




	auto jumpState = std::make_shared<FAnimState>();
	this->stateTable.push_back(jumpState);

	jumpState->setTexture_fn = ownerComp->setTexture_fn;

	std::vector<SDL_Texture*> jump_anims = {
game->GetTexture("Assets/Character07.png"),
game->GetTexture("Assets/Character08.png"),
game->GetTexture("Assets/Character09.png"),
game->GetTexture("Assets/Character10.png"),
game->GetTexture("Assets/Character11.png"),
game->GetTexture("Assets/Character12.png"),
	};

	jumpState->animClip = std::make_shared<FAnimClip>();
	jumpState->animClip->mAnimTextures = jump_anims;


	//hardcoded
	auto transition = Transition{ 0,1, ownerComp->jumpCond };
	this->transitions.push_back(transition);
 

	//auto transition2 = Transition{ 1,0, []() {return false;} };
	//this->transitions.push_back(transition2);


}

void PlayerController::Update(float dt)
{
	for (auto& trans : transitions) {
		trans.cond = mOwnerComp->jumpCond;
	}


	FSMController::Update(dt); 
}
