#pragma once


#pragma once
#include <cstdint>
#include <vector>
#include <iostream> 

namespace Config {

	inline constexpr uint32_t MAX_ACTORS = 20;
}
using namespace Config;

using namespace std;




#pragma once
#include <cstdint>
#include <vector>
#include <iostream> 

namespace Config {

	inline constexpr uint32_t MAX_ACTORS = 20;
}
using namespace Config;

using namespace std;


class CompAllocator {
public:
	virtual ~CompAllocator() = default;

	virtual void addComponent(uint32_t id) = 0;
	virtual void removeComponent(uint32_t id) = 0;


};


constexpr uint32_t INVALID_COMP_ID = 10000;

template<typename Value_t>
class CompAllocatorImpl : public CompAllocator {
public:
	virtual ~CompAllocatorImpl() = default;
	CompAllocatorImpl() {
		actorToComp = vector<uint32_t>(MAX_ACTORS, INVALID_COMP_ID);
		compToActor.reserve(MAX_ACTORS);
	}

	virtual void addComponent(uint32_t id) override
	{
		if (actorToComp[id] != INVALID_COMP_ID)
		{
			cout << "actor already has this component" << '\n';
			return;
		}
		else
		{
			denseData.push_back(Value_t{});
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
	vector<Value_t> denseData;

};



class CompAllocator {
public:
	virtual ~CompAllocator() = default;
	virtual void addComponent(uint32_t id) = 0;
	virtual void removeComponent(uint32_t id) = 0;
};


template<typename Derived_t>
class CompAllocatorImpl : public CompAllocator {
public:
	virtual ~CompAllocatorImpl() = default;
	CompAllocatorImpl() {
		actorToComp = vector<uint32_t>(MAX_ACTORS, 0);
		compToActor.reserve(MAX_ACTORS);
	}
	vector<uint32_t> actorToComp; //or sparseList
	vector<uint32_t> compToActor;  //or denseList  
	vector<Derived_t> denseData;

	virtual void addComponent(uint32_t id) override
	{
		if (actorToComp[id])
		{
			cout << "actor already has this component" << '\n';
			return;
		}
		else
		{
			denseData.push_back(Derived_t{});
			actorToComp[id] = static_cast<uint32_t>(denseData.size());
			compToActor.push_back(id);
		}
	}

	virtual void removeComponent(uint32_t id) override
	{
		if (actorToComp[id])
		{
			uint32_t compToRemoveId = actorToComp[id];

			//swap with last element
			uint32_t lastCompId = static_cast<uint32_t>(denseData.size()) - 1;
			swap(denseData[compToRemoveId], denseData[lastCompId]);
			swap(compToActor[compToRemoveId], compToActor[lastCompId]);

			//delete last element
			denseData.pop_back();
			compToActor.pop_back();

			//set actorToComp to 0
			actorToComp[id] = 0;
		}
		else
		{
			cout << "actor does not have this component" << '\n';
		}
	}
};

// Derived allocators
template<typename T>
struct CompAllocatorFinal : public CompAllocatorImpl<CompAllocatorFinal<T>> {





};
unordered_map<type_index, unique_ptr < void, void(*)(void*)>> componentAllocators;

template<typename T>
void registerComponent() {
	componentAllocators[typeid(T)] = std::unique_ptr<void, void(*)(void*)>(
		new CompAllocator<T>(),
		[](void* ptr) {
			delete static_cast<CompAllocator<T>*>(ptr);
		}
	);
}
template<typename T>
void addComponent(uint32_t id, T& comp)
{
	auto& allocator = *static_cast<CompAllocator<T>*>(componentAllocators[typeid(T)].get());
	allocator.addComponent(id, comp);

}