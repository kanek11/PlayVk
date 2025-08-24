#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h"

#include "PhysicsSync.h"

#include "Delegate.h"
//design decision: use PBD solver ;

//using namespace DirectX;

/*
design decision : a rigidbody is optional;
if the collider holds a weak ref of rb,  it directly communicate to it, and nothing more;
*/
//struct SleepParams {
//	float vLinearThreshold = 0.03f;   //  m/s
//	float vAngularThreshold = 2.0f;    //  deg/s 
//
//	//a bit higher
//	float wakeVLinear = 0.05f;
//	float wakeVAngular = 4.0f;
//
//	int FramesRequired = 40;
//
//	// optional
//	float wakeImpulseThreshold = 0.5f;
//	float wakeForceThreshold = 5.0f;
//
//	// RMS lowpass; smaller means smoother, but slower to respond
//	float emaBeta = 0.2f;
//};

struct PhysicalMaterial {
	float restitution;
	float friction;
};

struct RigidBody {
	RigidBody();

	float mass = 1.0f;   //or inverseMass
	float invMass = 1.0f; //inverse mass, 0 means infinite mass (static body)

	Float3 position;
	Float3 prevPos; //for previous
	Float3 predPos; //for prediction

	Float3 linearVelocity;
	Float3 force;  //accumulation in the frame;  

	//ref:  almost 0.000001f;  super elastic  0.001f
	float compliance = 0.000001f;

	float prevLinearSpeed{ 0.0f };
	float linearAccel; //approx;

	bool simulatePhysics{ false };
	PhysicalMaterial material{ 0.0f, 0.0f };

	bool simulateRotation{ false };
	DirectX::XMVECTOR rotation{ DirectX::XMQuaternionIdentity() };
	Float3 angularVelocity;
	Float3 torque{};

	Float3x3 RotationMatrix;

	//inertia
	Float3x3 localInertia;
	Float3x3 worldInertia;
	Float3x3 invWorldInertia;

	DirectX::XMVECTOR prevRot{ DirectX::XMQuaternionIdentity() };
	DirectX::XMVECTOR predRot{ DirectX::XMQuaternionIdentity() };


	//bool isKenematic;  
	void ApplyForceRate(const Float3& forceRate) {
		this->force += forceRate * 60.0f;
	}

	void ApplyImpulse(const Float3& impulseRate) {
		this->linearVelocity += impulseRate * 60.0f;
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

		//update rotation matrix:
		this->RotationMatrix = MMath::QuaternionToRotationMatrix(rotation);
	}

	void ClearRotation() {
		this->SetRotation(DirectX::XMQuaternionIdentity());

		angularVelocity *= 0.0f;
	}

	void SetPhysicalMaterial(const PhysicalMaterial& material) {
		this->material = material;
	}

	ShapeType type;
	void SetShape(ShapeType shape) {
		this->type = shape;
		localInertia = MakeInertiaTensor(shape, mass);
	}
	//new: set mass before reset shape;  for correct inertia
	void SetMass(float mass) {
		this->mass = mass;
		this->invMass = 1 / mass;
	}

	bool bFastStable{ true };
	float linearDamping = 0.999f;
	float angularDamping = 0.95f;

	//SleepParams sleepParams{};
	//bool   isSleeping = false;
	//int    sleepCounter = 0; 
};


struct Collider {
	Collider(RigidBody* body)
		:body(body)
	{
	};


	ShapeType type;
	AABB aabb; //for broadphase

	RigidBody* body{ nullptr };

	void SetShape(ShapeType shape) {
		this->type = shape;
	}

	void SetIsTrigger(bool isTrigger) {
		this->bIsTrigger = isTrigger;
	}

	ActorId actorId;
	bool bIsTrigger{ false };
	bool bEnabled{ true };
	bool bNeedsEvent{ false };
};

struct Contact {
	Float3 point;
	Float3 normal;
	float penetration;

	Collider* a{};
	Collider* b{};

	float lambda = 0.f;   // to restore , eg: force;


};


struct WorldShapeProxy;
using ColliderPair = std::pair<Collider*, Collider*>;
class BroadPhase {
public:
	void ComputePairs(std::vector<WorldShapeProxy>& ws, std::vector<ColliderPair>& out);

};


//class NarrowPhaseCollision {
//public:
//	void DetectCollisions();
//};

//class ContactSolver {
//public:
//	void ResolveContacts(std::span<Contact> contacts, float deltaTime);
//};
//
//class Integrator {
//public:
//	void Integrate();
//};



class PhysicsScene {
public:
	void Tick(float delata);

	void OnInit();
	void OnDestroy();


public:
	void AddRigidBody(RigidBody* rb,
		ActorId owner,
		const Float3& position = Float3{ 0.0f, 0.0f, 0.0f },
		const DirectX::XMVECTOR& rotation = DirectX::XMQuaternionIdentity()
	);

	void AddCollider(Collider* collider, ActorId owner);

	void RemoveRigidBody(ActorId owner);

	void RemoveCollider(ActorId owner);

	void SetShape(ActorId owner, ShapeType shape);
	void SetColliderShape(ActorId owner, ShapeType shape);

	void ClearRigidBodySync() {
		m_bodies.clear();
		m_transformBuffer.Clear();
		//m_commandBuffer.Enqueue([=]() {
		//	m_bodies.clear();
		//	m_transformBuffer.Clear();
		//	}); 
	}

	void ClearColliderSync() {
		m_colliders.clear();

		//m_commandBuffer.Enqueue([=]() {
		//	m_colliders.clear();
		//	}); 
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
	void PostSimulation(float delta);

private:
	std::unordered_map<ActorId, RigidBody*> m_bodies;
	std::unordered_map<ActorId, Collider*> m_colliders;
	//std::vector<Collider* > m_colliders;
	std::vector<Contact>  m_contacts;
	//std::vector<Constraints* > m_constraints;

	BroadPhase m_broadPhase;
	//ContactSolver m_contactSolver;
	//Integrator m_integrator;


public:
	Float3 gravity{ 0.0f, -9.8f, 0.0f };
	//Float3 gravity{ 0.0f, 0.0f, 0.0f }; 


public:
	PhysicsTransformBuffer& GetTransformBuffer() {
		return m_transformBuffer.GetReadBuffer();
	}

	void SetPosition(ActorId handle,Float3 position);
	void SetRotation(ActorId handle,DirectX::XMVECTOR rotation);

	void ClearBufferSync() {
		m_commandBuffer.Clear(); 
	}

private:
	PhysicsTransformSyncBuffer m_transformBuffer;
	PhysicsCommandBuffer m_commandBuffer;

};