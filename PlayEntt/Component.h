#pragma once
#include <cstdint>
#include <vector>
#include <iostream> 
#include <concepts>
#include <limits>


/*
* what must be done with specific type,  use type-erasure& template ;
* what should be common across any type, use virtual;

*/

namespace Config {

	inline constexpr uint32_t MAX_ACTORS = 100;
}
using namespace Config;

using namespace std;


 

constexpr uint32_t INVALID_COMP_ID = std::numeric_limits<uint32_t>::max();


// Base class for all component storages
class CompStorageBase {
public:
	virtual ~CompStorageBase() = default;
	virtual void removeComponent(uint32_t id) = 0; 
};

template<typename Comp_t>
class CompAllocator:  public CompStorageBase  {
public:  
	~CompAllocator() = default;
	CompAllocator() {
		actorToComp = vector<uint32_t>(MAX_ACTORS, INVALID_COMP_ID);
		compToActor.reserve(MAX_ACTORS);   
	}

	void addComponent(const uint32_t id, const Comp_t& comp) 
	{
		if (actorToComp[id] != INVALID_COMP_ID)
		{
			cout << "actor already has this component" << '\n';
			return;
		}
		else
		{
			denseData.push_back(comp);
			actorToComp[id] = static_cast<uint32_t>(denseData.size() - 1);
			compToActor.push_back(id);
		}
	}

	virtual void removeComponent(uint32_t id) override
	{
		if (actorToComp[id] != INVALID_COMP_ID)
		{
			uint32_t compToRemoveId = actorToComp[id]; 
			//cout << "to remove: " << compToRemoveId << '\n';

			//swap with last element
			uint32_t lastCompId = static_cast<uint32_t>(denseData.size()) - 1;
			swap(denseData[compToRemoveId], denseData[lastCompId]);
			swap(compToActor[compToRemoveId], compToActor[lastCompId]);

			//delete last element
			denseData.pop_back();
			compToActor.pop_back();

			//set actorToComp to 0
			actorToComp[id] = INVALID_COMP_ID;
		}
		else
		{
			cout << "actor does not have this component" << '\n';
		}
	}

	vector<uint32_t> actorToComp; //or sparseList
	vector<uint32_t> compToActor;  //or denseList  
	vector<Comp_t> denseData;

};
 

//template <typename Comp_t>
//concept is_comp = requires(Comp_t comp) {
//  { comp.addComponent() } -> std::same_as<void>;
//  { comp.actorToComp } -> std::same_as<vector<uint32_t>>;
//  { comp.compToActor } -> std::same_as<vector<uint32_t>>;
//  { comp.denseData } -> std::same_as<vector<Comp_t>>;
//};
