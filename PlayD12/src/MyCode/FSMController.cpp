#include "FSMController.h"

#include <iostream>
#include <format>

FSMController::FSMController()
{
	//define one default entry ? todo: a default transitionbehavior;
	//auto entryState = std::make_shared<FState>();
	//this->stateTable.push_back(entryState);
}

void FSMController::Update(float dt)
{
	this->stateTable[currentID]->OnStateUpdate(dt);

	for (auto& trans : this->transitions) {

		//std::cout << std::format("trans from:{},current:{},cond:{}", trans.from, currentID, trans.cond()) << '\n';
		
		if (trans.from == currentID && trans.cond()) {

			stateTable[currentID]->OnStateExit();
			currentID = trans.to;
			stateTable[currentID]->OnStateEnter();
			break;
		}
	}

}

FState::FState(std::string _name):
	name(_name)
{
}
