#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h"

//design decision: use PBD solver ;
 
using namespace DirectX;



struct PhysicalMaterial { 
	float restitution;
	float friction;
};

struct StaticMeshObjectProxy;

struct RigidBody {

	std::string debugName; //for debug purpose

	float mass = 1.0f;   //or inverseMass
	float invMass = 1.0f; //inverse mass, 0 means infinite mass (static body)

	FLOAT3 position;
	FLOAT3 prevPos; //for previous
	FLOAT3 predPos; //for prediction

	FLOAT3 linearVelocity;
	FLOAT3 force;  //accumulation in the frame; 

	bool simulatePhysics{ true };
	PhysicalMaterial material{ 0.0f,0.0f };

	bool simulateRotation{ false }; 
	XMVECTOR rotation{ XMQuaternionIdentity() }; 
	FLOAT3 angularVelocity;
	FLOAT3 torque{};

	FLOAT3X3 RotationMatrix; 

	//inertia
	FLOAT3X3 localInertia;   
	FLOAT3X3 worldInertia;  
	FLOAT3X3  invWorldInertia;

	XMVECTOR prevRot{ XMQuaternionIdentity() };
	XMVECTOR predRot{ XMQuaternionIdentity() };

	ShapeType type;

	//bool isKenematic;  
	void ApplyForce(FLOAT3 force) {
		this->force += force;
	}

	void ApplyTorque(const FLOAT3& torque) {
		this->torque += torque;
	}

	//todo: angular;
	StaticMeshObjectProxy* owner;
	RigidBody(StaticMeshObjectProxy* owner, FLOAT3 position,ShapeType type, XMVECTOR rotation)
		: owner(owner), position(position), type(type), rotation(rotation)
	{
		 
		predPos = position;  
		prevPos = position; 

		predRot = rotation;
		prevRot = rotation; 

		localInertia = MakeInertiaTensor(type, 10.0f); 
		//std::cout << "RigidBody created: " << typeid(type).name() << std::endl;
		XMMATRIX R_ = XMMatrixRotationQuaternion(rotation);
		FLOAT3X3 R;
		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };
		RotationMatrix = R;
	};
};

/*
design decision : a rigidbody is optional;
if the collider holds a weak ref of rb,  it directly communicate to it, and nothing more;
*/


struct Collider {
	ShapeType type; 

	RigidBody* body{ nullptr };
	StaticMeshObjectProxy* owner;
	Collider(StaticMeshObjectProxy* owner, ShapeType type, RigidBody* body)
		: owner(owner), type(type), body(body)
	{
	};
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
	void AddRigidBody(RigidBody* rb);
	void AddCollider(Collider* collider) {
		m_colliders.push_back(collider);
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
	FLOAT3 gravity{ 0.0f, -9.8f, 0.0f };
};