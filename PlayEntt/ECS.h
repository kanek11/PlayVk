#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream> 
#include <typeindex>
#include <memory>
#include <optional>
#include <span>
#include <tuple>

#include "Component.h"

using namespace std;

 

//struct Actor
//{
//	uint32_t id;
//};

 

class ActorManager
{
public:
	vector<bool> sparseList;
	vector<uint32_t> freeList;  

	ActorManager()
	{
		sparseList = vector<bool>(MAX_ACTORS, false);

		freeList.reserve(MAX_ACTORS);  
		for (uint32_t i = 0; i < MAX_ACTORS; i++)
		{
			freeList.push_back(MAX_ACTORS - i - 1);
		} 
	}

	uint32_t createActor()
	{
		uint32_t id{};
		if (freeList.size() > 0)
		{
			id = freeList.back();
			freeList.pop_back();

			sparseList[id] = true;
		}
		else
		{
			cout << "run out of object pool" << '\n';
		}
		return id;
	}


	void removeActor(uint32_t id)
	{
		freeList.push_back(id);  
		sparseList[id] = false;
	}

	bool isAlive(uint32_t id) { return sparseList[id]; }
	 
};

 

class ECS { 
public:
	~ECS() = default;
	ECS() = default;

	ActorManager actorManager{};

	// shared ptr will capture the correct destructor; 
	unordered_map<type_index, shared_ptr<CompStorageBase>> componentAllocators;


public:
	uint32_t createActor()
	{
		return actorManager.createActor(); 
	}

 template <typename Comp_t> 
	void registerComponent() {
		auto typeIdx = type_index(typeid(Comp_t));

	   if (componentAllocators.contains(typeIdx)) {
		 cerr << "component type already registered!" << '\n';
		 return;
	   }
	    componentAllocators[typeIdx] = make_shared<CompAllocator<Comp_t>>();
	}

	template<typename Comp_t> 
	void addComponent(const uint32_t id, const std::optional<Comp_t>& comp = std::nullopt)
	{ 
		if (!actorManager.isAlive(id)) {
            cout << "actor is not active" << '\n';
            return;
        }

		if (componentAllocators.contains(typeid(Comp_t))) {

            auto allocator = componentAllocators[typeid(Comp_t)].get();
            auto sparseSet = static_cast<CompAllocator<Comp_t>*>(allocator);
            sparseSet->addComponent(id, comp.value_or(Comp_t{})); 
        }
		else
		{
			cerr << "component type not registered!" << '\n';
		}

	} 


	void removeActor(uint32_t id)
	{
          if (!actorManager.isAlive(id)) {
            cerr << "actor is not active" << '\n';
            return;
          }

		//cout << "to remove" << id << '\n';

		actorManager.removeActor(id); 
		// delete all components  
		for (auto& [type, allocator] : componentAllocators)
		{ 
			allocator->removeComponent(id);
		}

	} 
	
	template<typename Comp_t> 
	std::tuple<span<uint32_t>,span<Comp_t>> getComponentViews()
	{
        if(componentAllocators.contains(typeid(Comp_t) ) )
		{
            auto allocator = componentAllocators[typeid(Comp_t)].get();
			auto sparseSet = static_cast<CompAllocator<Comp_t>*>(allocator);
			return { 
				span<uint32_t>(sparseSet->compToActor),
				span<Comp_t>(sparseSet->denseData) };
		}
		else
		{
			cerr << "component not registered!" << '\n'; 
			return {};
		}

		
	}

};