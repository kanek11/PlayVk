#include "AnimComponent.h"



FAnimComponent::FAnimComponent(Actor* owner, int drawOrder):
	Component(owner,drawOrder)
{
}

void FAnimComponent::Update(float dt)
{
	controller->Update(dt) ;
}
