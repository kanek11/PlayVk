#include "PCH.h"
#include "PhysicsScene.h"

#include "Render/Renderer.h" 

#include "Collision.h" 


void PhysicsScene::Tick(float delta)
{
	//std::cout << "tick physics: " << delta << '\n';

	PreSimulation();

	ApplyExternalForce(delta);

	IntegrateVelocity(delta);

	DetectCollisions();

	ResolveContacts(delta);

	IntegratePosition(delta);

	PostSimulation();
}

void PhysicsScene::OnInit()
{
}

void PhysicsScene::OnDestroy()
{
}

void PhysicsScene::AddRigidBody(RigidBody* rb)
{
	this->m_bodies.push_back(rb);
}

void PhysicsScene::PreSimulation()
{
	//for (auto& collider : m_colliders) {
	//	//collider->GetWorldPosition(); 
	//}
}

void PhysicsScene::ApplyExternalForce(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->ApplyForce(this->gravity);
	}
}

void PhysicsScene::IntegrateVelocity(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->linearVelocity = rb->linearVelocity + rb->force / rb->mass * delta ;
		//rb->linearVelocity *= 0.99f;  //test damping;
		rb->force = FLOAT3{};
	}
}




void PhysicsScene::DetectCollisions()
{
	m_contacts.clear();

	//std::cout << "Detecting collisions, colliders count: " << m_colliders.size() << std::endl;

	std::vector<WorldShapeProxy> ws;
	ws.reserve(m_colliders.size());
	for (Collider* c : m_colliders) {
		WorldShapeProxy proxy = { MakeWorldShape(*c), c };
		ws.push_back(proxy);
	}

	for (size_t i = 0; i < ws.size(); ++i)
		for (size_t j = i + 1; j < ws.size(); ++j)
		{
			auto& A = ws[i];
			auto& B = ws[j];

			std::visit([&, this](auto const& sa, auto const& sb)
				{
					Contact c;
					c.a = A.owner;
					c.b = B.owner;
					if (Collide(sa, sb, c)) {
						//std::cout << "Collision detected: " << typeid(decltype(sa)).name() << " vs " << typeid(decltype(sb)).name() << std::endl;

						m_contacts.emplace_back(std::move(c));
					}

				}, A.shape, B.shape);
		}

	//std::cout << "Contacts detected: " << m_contacts.size() << std::endl;

}

void PhysicsScene::ResolveContacts(float dt)
{
	//Baumgarte stabilization;
	constexpr float slop = 0.005f;      // simply penetration allowance
	constexpr float beta = 0.2f;        // Baumgarte factor; 0~1 and bias to 0;
	constexpr float bounceThreshold = 1.0f;    // m/s
	constexpr float restingThreshold = 0.01f;  // m/s
	constexpr int   iterations = 1;            // solver passes

	for (int pass = 0; pass < iterations; ++pass) {
		for (Contact& contact : m_contacts) {
			RigidBody* A = contact.a->body;
			RigidBody* B = contact.b->body;

			float invMassA = (A && A->mass > 0.f) ? 1.f / A->mass : 0.f;
			float invMassB = (B && B->mass > 0.f) ? 1.f / B->mass : 0.f;
			float invMassSum = invMassA + invMassB;
			if (invMassSum == 0.f) continue;

			// B 2 A 
			FLOAT3 normal = contact.normal;

			// --- relative vel
			FLOAT3 velA = A ? A->linearVelocity : FLOAT3{ 0,0,0 };
			FLOAT3 velB = B ? B->linearVelocity : FLOAT3{ 0,0,0 };
			FLOAT3 relVel = velA - velB;            

			//normal component ;  
			//bug fix: approaching if negative;  
			float vn_mag = Dot(relVel, normal);    
			 

			// --- 3) Baumgarte positional bias ---
			float penetration = std::max(contact.penetration - slop, 0.f);
			float bias = (beta / dt) * penetration;  // positive = push fast

		 
			//Coefficient of restitution
			//a perfectly elastic will flip the velocity.
			float eA = A ? A->material.restitution : 0.f;
			float eB = B ? B->material.restitution : 0.f;
			float e = std::min(eA, eB);
			float rt_Impulse = (vn_mag < -bounceThreshold) ? e : 0.f;

			//set 0 when almost static
			if (std::abs(vn_mag) < restingThreshold) vn_mag = 0.f; 

			//impulse scalar j
			float jn = 0.f;

			// approaching
			if (vn_mag < 0.f) {
				jn = -(1.f + rt_Impulse) * vn_mag + bias;
				jn /= invMassSum;
			}
			//already separate
			else {
				continue; 
			}

			// apply impulse on normal direction
			FLOAT3 impulseN = normal * jn;
			if (A) A->linearVelocity += impulseN * invMassA;
			if (B) B->linearVelocity -= impulseN * invMassB;


			//tangent direction:
			FLOAT3 vt = relVel - normal * vn_mag; 

			//end loop when too small;
			if (LengthSq(vt) < 1e-6f)  continue;

			FLOAT3 vt_unit = Normalize(vt); 
			float jt = -Dot(relVel, vt_unit) / invMassSum; 

			//Coulomb's friction model:  friction = mu * normal force; 
			float muA = A ? A->material.friction : 0.f;
			float muB = B ? B->material.friction : 0.f;
			float mu = std::sqrt(muA * muB);
			float jtMax = mu * jn;
			jt = std::clamp(jt, -jtMax, jtMax);
			
			FLOAT3 impulseT = vt_unit * jt;
			if (A) A->linearVelocity += impulseT * invMassA;
			if (B) B->linearVelocity -= impulseT * invMassB; 
		}
	}
}

void PhysicsScene::IntegratePosition(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->position = rb->position + rb->linearVelocity * delta ;
	}
}

void PhysicsScene::PostSimulation()
{
	for (auto& rb : m_bodies) {
		rb->owner->SetWorldPosition(rb->position);
	}
}