#pragma once


#include "PhysicsSync.h"
#include "PhysicsScene.h"

//using OnOverlap = FDelegate<void(Contact)>;
struct FCollisionEvent {
    ActorId a_ID;
    ActorId b_ID;
    Contact contact;
};


class PhysicsEventQueue {
public:
    static PhysicsEventQueue& Get() {
        static PhysicsEventQueue instance;
        return instance;
    }

    void Push(const FCollisionEvent& event) {
        std::lock_guard<std::mutex> lock(mutex);
        events.push_back(event);
    }

    std::vector<FCollisionEvent> Drain() {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<FCollisionEvent> out = std::move(events);
        events.clear();
        return out;
    }

private:
    std::vector<FCollisionEvent> events;
    std::mutex mutex;
};