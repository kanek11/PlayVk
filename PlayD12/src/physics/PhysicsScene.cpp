#include "PCH.h"
#include "PhysicsScene.h"

#include "Render/Renderer.h" 

#include "Collision.h" 


void PhysicsScene::Tick(float delta)
{
	//std::cout << "tick physics: " << delta << '\n';

	constexpr int   iterations = 1;            // solver passes


	PreSimulation();

	ApplyExternalForce(delta);

	Integrate(delta);

	DetectCollisions();

	SolveConstraints(delta);

	PostPBD(delta);
	//IntegratePosition(delta);

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
	for (auto& rb : m_bodies) {

		//cache the previous position:
		rb->prevPos = rb->position; 


		if (!rb->enableRotation) continue;
		XMMATRIX R_ = XMMatrixRotationQuaternion(rb->rotation);
		FLOAT3X3 R;
		R[0] = { R_.r[0].m128_f32[0], R_.r[1].m128_f32[0], R_.r[2].m128_f32[0] };
		R[1] = { R_.r[0].m128_f32[1], R_.r[1].m128_f32[1], R_.r[2].m128_f32[1] };
		R[2] = { R_.r[0].m128_f32[2], R_.r[1].m128_f32[2], R_.r[2].m128_f32[2] };

		auto temp = MatrixMultiply(R, rb->localInertia);
		auto R_t = Transpose(R);
		auto worldInertia = MatrixMultiply(temp, R_t);
		rb->worldInertia = worldInertia;
		rb->RotationMatrix = R;
	}
}

void PhysicsScene::ApplyExternalForce(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->ApplyForce(this->gravity);
		 
	}
}

void PhysicsScene::Integrate(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;

		rb->linearVelocity = rb->linearVelocity + rb->force / rb->mass * delta ;
		rb->force = FLOAT3{};
		 
		//predicated position:
		rb->predPos = rb->position + rb->linearVelocity * delta;   
		 

		//new: consider torque:
		if (!rb->enableRotation) continue; //skip if not enabled
		rb->angularVelocity = rb->angularVelocity + Inverse(rb->worldInertia) * rb->torque * delta; 
		rb->torque = FLOAT3{}; //reset torque
		 
		//angular damping: 
//	rb->angularVelocity *= 0.98f; //test damping;
//} 
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

void PhysicsScene::SolveConstraints(float delta)
{ 
	//hardcode compliance:
	constexpr float compliance = 0.0001f; // compliance factor 
	const float inv_dt2 = 1.f / (delta * delta); // 1 / dt²
	 
	for (Contact& contact : m_contacts) {
		contact.lambda = 0.f;
	}
	 
	for (Contact& contact : m_contacts) {
		RigidBody* A = contact.a->body;
		RigidBody* B = contact.b->body;

		FLOAT3 posA = A ? A->predPos : FLOAT3{}; 
		FLOAT3 posB = B ? B->predPos : FLOAT3{};  

		float  wA = A && A->simulatePhysics ? A->invMass : 0.f;  
		float  wB = B && B->simulatePhysics ? B->invMass : 0.f;  
		float  wSum = wA + wB;
		if (wSum == 0) continue;

		float C = contact.penetration;  
		if (C <= 0) { 
			continue; 
		}  
		//if (C <= 0.01f ) continue;
		// XPBD: α = compliance / dt²
		float alpha = compliance * inv_dt2;
		float dLambda = (C + alpha * contact.lambda) / (wSum + alpha);
		contact.lambda += dLambda;

		FLOAT3 corr = dLambda * contact.normal;
		//FLOAT3 corr = C * contact.normal / wSum; // correction vector
	 
		//apply correction to predicted positions:
		if (A) A->predPos += corr * wA; // 
		if (B) B->predPos -= corr * wB; //  


		//post PBD: 
		//FLOAT3 vA = A ? (A->predPos - A->prevPos) / delta : FLOAT3{}; // predicted velocity
		//FLOAT3 vB = B ? (B->predPos - B->prevPos) / delta : FLOAT3{}; // predicted velocity
		//
		//FLOAT3 v_rel = vA - vB;
		//float vn = Dot(v_rel, contact.normal);
		//if (vn >= 0) continue; // no need to resolve if they are separating
 	// 
		//float eA = A ? A->material.restitution : 0.f;
		//float eB = B ? B->material.restitution : 0.f;
		//float restitution = std::min(eA, eB);  

		//float threshold = 0.01f; // threshold for bounce, can be adjusted
		////if (vn < -threshold) { 
		//	float vBounce = -vn * restitution;
		//	FLOAT3 corr_n = vBounce * contact.normal * delta / wSum;

		//	// Apply bounce as position shift 
		//	if (A) A->predPos += corr_n * wA; 
		//	if (B) B->predPos -= corr_n * wB; 
		////}

	} 
}

void PhysicsScene::PostPBD(float delta)
{
	//pbd step after solving constraints:
	//v = (x - x0) / dt
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;  

		rb->position = rb->predPos; //update position to predicted position 
		rb->linearVelocity = (rb->predPos - rb->prevPos) / delta;
		  
		//rb->linearVelocity *= 0.99f;  //test damping;
		if (LengthSq(rb->linearVelocity) < 1e-6f)  
			rb->linearVelocity = FLOAT3{}; //reset to zero if too small
	} 

}

void PhysicsScene::IntegratePosition(float delta)
{
	for (auto& rb : m_bodies) {
		if (!rb->simulatePhysics) continue;
		//rb->position = rb->position + rb->linearVelocity * delta ;


		if (!rb->enableRotation) continue;
        //rotation by quaternion: 
		XMVECTOR q = rb->rotation; // 假设为 XMVECTOR 类型（x,y,z,w）

		XMVECTOR omegaQuat = XMVectorSet(
			rb->angularVelocity.x(),
			rb->angularVelocity.y(),
			rb->angularVelocity.z(),
			0.0f
		);

		// dq = 0.5 * omegaQuat * q
		XMVECTOR dq = XMQuaternionMultiply(omegaQuat, q);
		dq = XMVectorScale(dq, 0.5f);
		 
		XMVECTOR updatedRot = XMVectorAdd(q, XMVectorScale(dq, delta));
		rb->rotation = XMQuaternionNormalize(updatedRot);
	}
}

void PhysicsScene::PostSimulation()
{
	for (auto& rb : m_bodies) {
		rb->owner->SetWorldPosition(rb->position);

		//std::cout << "Position vs PredPos: " << rb->position.y() << " vs " << rb->predPos.y() << std::endl;

		if (!rb->enableRotation) continue;  
		rb->owner->SetWorldRotation(rb->rotation);
		//update world inertia:

	}
}