#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h"

#include "Gameplay/Actor.h"

#include "PhysicsSync.h"
//design decision: use PBD solver ;

//using namespace DirectX;

/*
design decision : a rigidbody is optional;
if the collider holds a weak ref of rb,  it directly communicate to it, and nothing more;
*/



struct PhysicalMaterial {
	float restitution;
	float friction;
};

struct StaticMeshActorProxy;

struct RigidBody {

	std::string debugName; //for debug purpose

	float mass = 1.0f;   //or inverseMass
	float invMass = 1.0f; //inverse mass, 0 means infinite mass (static body)

	Float3 position;
	Float3 prevPos; //for previous
	Float3 predPos; //for prediction

	Float3 linearVelocity;
	Float3 force;  //accumulation in the frame; 

	bool simulatePhysics{ false };
	PhysicalMaterial material{ 0.0f, 0.8f };

	bool simulateRotation{ false };
	DirectX::XMVECTOR rotation{ DirectX::XMQuaternionIdentity() };
	Float3 angularVelocity;
	Float3 torque{};

	Float3x3 RotationMatrix;

	//inertia
	Float3x3 localInertia;
	Float3x3 worldInertia;
	Float3x3  invWorldInertia;

	DirectX::XMVECTOR prevRot{ DirectX::XMQuaternionIdentity() };
	DirectX::XMVECTOR predRot{ DirectX::XMQuaternionIdentity() };


	//bool isKenematic;  
	void ApplyForceRate(Float3 forceRate) {
		this->force += forceRate * 60.0f;
	}

	void ApplyTorque(const Float3& torque) {
		this->torque += torque;
	} 
 
	void SetPosition(const Float3& position) {
		this->position = position;
		this->predPos = position;
		this->prevPos = position;
	}

	void SetRotation(const DirectX::XMVECTOR& rotation) {
		this->rotation = rotation;
		this->predRot = rotation;
		this->prevRot = rotation;
	}

	ShapeType type;
	void SetShape(ShapeType shape) {
		this->type = shape;
		localInertia = MakeInertiaTensor(shape, mass); 
	}

	RigidBody();
};


struct Collider {
	ShapeType type;

	RigidBody* body{ nullptr }; 
	Collider(ShapeType type, RigidBody* body)
		:type(type), body(body)
	{
	};
};

//using ColliderPair = std::pair<const Collider* , const Collider* >;
//class BroadPhaseCollision {
//public:
//	 
//};


struct Contact {
	Float3 point;
	Float3 normal;
	float penetration;

	Collider* a{};
	Collider* b{};

	float  lambda = 0.f;   // to restore , eg: force;
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
 

class PhysicsScene {
public:
	void Tick(float delata);

	void OnInit();
	void OnDestroy();


public:
	void AddRigidBody(RigidBody* rb, 
		ActorHandle owener,
		const Float3& position = Float3{ 0.0f, 0.0f, 0.0f },
		const DirectX::XMVECTOR& rotation = DirectX::XMQuaternionIdentity()
	);
	void AddCollider(Collider* collider) {
		m_colliders.push_back(collider);
	}

	void ClearRigidBody() {
		m_bodies.clear();
	}

	void ClearCollider() {
		m_colliders.clear();
	}

private:
	void PreSimulation();

	//accumulate global / user-defined forces
	void ApplyExternalForce(float delta);

	//prediction
	void Integrate(float delta);

	//simulation:
	void DetectCollisions();

	void SolveConstraints(float delta);

	//todo:
	//void ResolveConstraints();


	void PostPBD(float delta);

	void VelocityPass(float delta);
	  
	//update again, signal events,  etc.
	void PostSimulation();

private:
	std::unordered_map<ActorHandle, RigidBody*> m_bodies;
	std::vector<Collider* > m_colliders;
	std::vector<Contact>  m_contacts;
	//std::vector<Constraints* > m_constraints;

	ContactSolver m_contactSolver;
	Integrator m_integrator;

	 
public:
	Float3 gravity{ 0.0f, -9.8f, 0.0f };
	//Float3 gravity{ 0.0f, 0.0f, 0.0f }; 


public:
	PhysicsTransformBuffer& GetTransformBuffer() {
		return m_transformBuffer.GetReadBuffer();
	}

	void SetPosition(ActorHandle handle, const Float3& position);
	void SetRotation(ActorHandle handle, const DirectX::XMVECTOR& rotation);

private:
	PhysicsTransformSyncBuffer m_transformBuffer; 
	PhysicsCommandBuffer m_commandBuffer;
};


