#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h"

//struct PhysicalMaterial { 
//	float restitution;
//	float friction;
//};
 
struct StaticMeshObjectProxy;

struct RigidBody {

	float mass = 1.0f;   //or inverseMass

	FLOAT3 position;
	FLOAT3 linearVelocity;
	FLOAT3 force;  //accumulation in the frame; 

	bool simulatePhysics{ true };

	//bool isKenematic;  
	void ApplyForce(FLOAT3 force) {
		this->force = force;
	}

	//todo: angular;
	StaticMeshObjectProxy* owner; 
	RigidBody(StaticMeshObjectProxy* owner, FLOAT3 position)
		: owner(owner), position(position)
	{};
};

/* 
design decision : a rigidbody is optional;
if the collider holds a weak ref of rb,  it directly communicate to it, and nothing more;
*/

using ShapeType = std::variant<Plane, Sphere, Box>;

struct Collider {
	ShapeType type;
	FLOAT3 localOffset;

	RigidBody* body{ nullptr };
	StaticMeshObjectProxy* owner;
	Collider(StaticMeshObjectProxy* owner, ShapeType type, RigidBody* body) 
		: owner(owner), type(type), body(body)
	{}; 
};




//using ColliderPair = std::pair<const Collider* , const Collider* >;
//class BroadPhaseCollision {
//public:
//	 
//};


struct Contact {
	FLOAT3 point;
	FLOAT3 normal;
	float penetration;

	Collider* a{};
	Collider* b{};
};


class NarrowPhaseCollision {
public:
	void DetectCollisions();
};

class ContactSolver {
public:
	void ResolveContacts(std::span<Contact> contacts, float deltaTime);
};
 
class Integrator {
public:
	void Integrate();
};
 

class PhysicalScene { 
public:
	void Tick(float delata);

	void OnInit(); 
	void OnDestroy(); 


public:
	void AddRigidBody(RigidBody* rb);
	void AddCollider(Collider* collider) {
		m_colliders.push_back(collider);
	}

private: 
	void PreSimulation();

	//accumulate global / user-defined forces
	void ApplyExternalForce(float delta);

	//prediction
	void IntegrateVelocity(float delta);

	//simulation:
	void DetectCollisions();
	 
	void ResolveContacts(float delta);

	//todo:
	//void ResolveConstraints();

	//
	void IntegratePosition(float delta);

	//update again, signal events,  etc.
	void PostSimulation(); 

private:
	std::vector<RigidBody*> m_bodies;
	std::vector<Collider* > m_colliders;
	std::vector<Contact>  m_contacts;
	//std::vector<Constraints* > m_constraints;

	ContactSolver m_contactSolver;
	Integrator m_integrator;


//experimental
public:
	FLOAT3 gravity{ 0.0f, 0.1f* -9.8f, 0.0f };
};