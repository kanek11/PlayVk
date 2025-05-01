#include <entt/entt.hpp>
#include <iostream>

struct position {
    float x;
    float y;
};Vie

struct velocity {
    float dx;
    float dy;
};

void SystemUpdate(entt::registry& registry) {
    auto view = registry.view<const position, velocity>();

    // use a callback
    view.each([](const auto& pos, auto& vel)  {   });

    // use an extended callback
    view.each([](const auto entity, const auto& pos, auto& vel) 
        { std::cout << "process entities:" << 
		"id:" << static_cast<uint32_t>(entity) << '\n';
        });

    // use a range-for
    for (auto [entity, pos, vel] : view.each()) {
        // ...
    }

    // use forward iterators and get only the components of interest
    for (auto entity : view) {
        auto& vel = view.get<velocity>(entity);
        // ...
    }

    for (auto entity : view) {
        auto& vel = view.get<position>(entity);
        // ...
    }
} 

int main() {
    entt::registry registry;

    for (auto i = 0u; i < 10u; ++i) {
        const auto entity = registry.create();
        registry.emplace<position>(entity, i * 1.f, i * 1.f);
        if (i % 2 == 0) { registry.emplace<velocity>(entity, i * .1f, i * .1f); }
    }

    SystemUpdate(registry);
}