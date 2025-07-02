#include "PCH.h"
#include "PhysicsScene.h"

#include "Render/Renderer.h"


#include "Collision.h" 


void PhysicalScene::Tick(float delta)
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

void PhysicalScene::OnInit()
{
}

void PhysicalScene::OnDestroy()
{
}

void PhysicalScene::AddRigidBody(RigidBody* rb)
{
	this->m_bodies.push_back(rb);
}

void PhysicalScene::PreSimulation()
{
	//for (auto& collider : m_colliders) {
	//	//collider->GetWorldPosition(); 
	//}
}

void PhysicalScene::ApplyExternalForce(float delta)
{ 
	for (auto& rb : m_bodies) { 
		if (!rb->simulatePhysics) continue;
		rb->ApplyForce(this->gravity);
	}
}

void PhysicalScene::IntegrateVelocity(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->linearVelocity = rb->linearVelocity + rb->force / rb->mass * delta;
		rb->force = FLOAT3{};
	}
}


 

void PhysicalScene::DetectCollisions()
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

	//
	// 
	std::cout << "Contacts detected: " << m_contacts.size() << std::endl;

}

void PhysicalScene::ResolveContacts(float dt)
{
	constexpr float slop = 0.005f;      // penetration allowance
	constexpr float beta = 0.2f;        // Baumgarte factor
	constexpr float bounceThreshold = 1.0f;    // m/s
	constexpr float restingThreshold = 0.01f;  // m/s
	constexpr int   iterations = 8;            // solver passes

	for (int pass = 0; pass < iterations; ++pass) {
		for (Contact& c : m_contacts) {
			RigidBody* A = c.a->body;
			RigidBody* B = c.b->body;
			float invMassA = (A && A->mass > 0.f) ? 1.f / A->mass : 0.f;
			float invMassB = (B && B->mass > 0.f) ? 1.f / B->mass : 0.f;
			float invMassSum = invMassA + invMassB;
			if (invMassSum == 0.f) continue;

			// --- 1) 统一法线方向（normal from B→A）---
			FLOAT3 n = c.normal;

			// --- 2) 计算相对速度：fix sign ---
			FLOAT3 velA = A ? A->linearVelocity : FLOAT3{ 0,0,0 };
			FLOAT3 velB = B ? B->linearVelocity : FLOAT3{ 0,0,0 };
			FLOAT3 relVel = velA - velB;             // was velB - velA
			float vn = Dot(relVel, n);               // 正值表示正在接近 (approaching)

			// --- 3) Baumgarte 位置修正 (positional bias) ---
			float penetration = std::max(c.penetration - slop, 0.f);
			float bias = (beta / dt) * penetration;  // positive → 推离更快

			// --- 4) 弹性系数 ---
			float restitution = 0.0f; // 弹性系数，0-1之间
			float e = (vn < -bounceThreshold) ? restitution : 0.f;
			// “静止”时不再反弹
			if (std::abs(vn) < restingThreshold) vn = 0.f;

			// --- 5) 计算脉冲强度 (impulse scalar) ---
			float j = 0.f;
			if (vn < 0.f) {
				// approaching
				j = -(1.f + e) * vn + bias;
				j /= invMassSum;
			}
			else {
				continue; // 分离或静止，无需处理
			}

			// --- 6) 应用脉冲 ---
			FLOAT3 impulse = n * j;
			if (A) A->linearVelocity += impulse * invMassA;
			if (B) B->linearVelocity -= impulse * invMassB;
		}
	}
}

void PhysicalScene::IntegratePosition(float delta)
{
	for (auto& rb : m_bodies) { 
		if (!rb->simulatePhysics) continue;
		rb->position = rb->position + rb->linearVelocity * delta;
	}
}

void PhysicalScene::PostSimulation()
{ 
	for (auto& rb : m_bodies) { 
		rb->owner->SetWorldPosition(rb->position); 
	}
}
