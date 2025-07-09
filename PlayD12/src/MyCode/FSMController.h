#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <string>
//using Cond_Fn 


class FState {
public:
	virtual ~FState() = default;
	FState(std::string name = std::string("Default State"));

	virtual void OnStateEnter() {};
	virtual void OnStateExit() {};
	virtual void OnStateUpdate(float dt) {};

	std::string name;
};

//template<typename T>
//class FState : public IState {
//public: 
//
//	//std::function<void(T)> SetPropertyFn;
//	T currentValue;
//};


class FSMController {
public:
	FSMController();
	~FSMController() = default;  

	virtual void Update(float dt);  
	

	struct Transition { size_t from, to;  std::function<bool()> cond; }; 

	std::vector<std::shared_ptr<FState>> stateTable;
	std::vector<Transition> transitions;

	size_t currentID{ 0 };
};

