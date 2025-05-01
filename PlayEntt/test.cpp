#include "ECS.h"

#include <algorithm>
using namespace std;

struct Pos {
	float x;
	float y;
};

struct Vel {
	float dx;
	float dy;
};





void updatePos(auto& ecs)
{
	auto [posActors, posView] = ecs.getComponentViews<Pos>();

	for (auto pos : posView) {
		cout << "pos: " << pos.x << ' ' << pos.y << '\n';
	} 
	for (auto actor : posActors) {
		cout << "actor: " << actor << '\n';
	}

}








int main()
{

	ECS ecs;
	ecs.registerComponent<Pos>();
	ecs.registerComponent<Vel>();
	
	auto actor0 = ecs.createActor();
	
	ecs.addComponent<Pos>(actor0);
	ecs.addComponent<Vel>(actor0);

	std::cout << "create actor" << actor0 << '\n';

	for (auto i = 1u; i < 10u; ++i) {
		const auto entity = ecs.createActor();
		ecs.addComponent<Pos>(entity);
		if (i % 2 == 0) { ecs.addComponent<Vel>(entity); }
	}

	std::cout << "active slots: \n";
	ranges::for_each(ecs.actorManager.sparseList, [](const auto& active) {
		std::cout << active << ' ';
		});

	std::cout << '\n';


	ecs.removeActor(actor0);

	cout << "active slots: " << '\n'; 
	ranges::for_each(ecs.actorManager.sparseList, [](const auto& active) {
		std::cout << active << ' ';
		});
	cout << '\n';


	updatePos(ecs);

}