#include "PCH.h"
#include "PhysicsScene.h"

#include "Render/Renderer.h" 

#include "Collision.h"  

using namespace DirectX;  

void PhysicsScene::Tick(float delta)
{
	//std::cout << "tick physics: " << delta << '\n';

	float substeps = 1;
	float substepDelta = delta / substeps;


	PreSimulation();

	ApplyExternalForce(delta);

	for (int i = 0; i < substeps; ++i) {

		Integrate(substepDelta);

		DetectCollisions();

		SolveConstraints(substepDelta);

		PostPBD(substepDelta);
		//IntegratePosition(delta); 

	}

	VelocityPass(delta);

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

		rb->linearVelocity = rb->linearVelocity + rb->force / rb->mass * delta;
		rb->force = FLOAT3{};

		//predicated position:
		rb->predPos = rb->position + rb->linearVelocity * delta;


		//new: consider torque:
		rb->angularVelocity = rb->angularVelocity + Inverse(rb->worldInertia) * rb->torque * delta;
		rb->torque = FLOAT3{}; //reset torque 

		rb->prevRot = rb->rotation;

		XMVECTOR dq = XMVectorSet(rb->angularVelocity.x(), rb->angularVelocity.y(), rb->angularVelocity.z(), 0.0f);
		dq = XMQuaternionMultiply(dq, rb->rotation);
		dq = XMVectorScale(dq, 0.5f * delta);

		rb->predRot += dq; 
		rb->predRot = XMQuaternionNormalize(rb->predRot);


		XMMATRIX R_ = XMMatrixRotationQuaternion(rb->prevRot);
		FLOAT3X3 R;
		//R[0] = { R_.r[0].m128_f32[0], R_.r[1].m128_f32[0], R_.r[2].m128_f32[0] };
		//R[1] = { R_.r[0].m128_f32[1], R_.r[1].m128_f32[1], R_.r[2].m128_f32[1] };
		//R[2] = { R_.r[0].m128_f32[2], R_.r[1].m128_f32[2], R_.r[2].m128_f32[2] };

		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };
		rb->RotationMatrix = R;

		//test for orthognol:
		//assert(Dot(R[0], R[1]) < 0.01f);
		//assert(Dot(R[1], R[2]) < 0.01f);
		//assert(Dot(R[0], R[2]) < 0.01f); 

		if (!rb->simulateRotation) continue;
		auto worldInertia = MatrixMultiply(MatrixMultiply(R, rb->localInertia), Transpose(R));
		rb->worldInertia = worldInertia;
		rb->invWorldInertia = Inverse(worldInertia);

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

	auto generalizedInvMass = [&](RigidBody* rb,
		const FLOAT3& ri,
		const FLOAT3& n)->float
		{
			if (!rb || !rb->simulatePhysics) return 0.f;

			if (rb->simulateRotation == false) {
				return rb->invMass;
			}
			FLOAT3 cross = Vector3Cross(ri, n);
			FLOAT3 tmp = rb->invWorldInertia * cross;
			return rb->invMass + Dot(cross, tmp);     // scalar
		};


	for (Contact& contact : m_contacts) {
		RigidBody* A = contact.a->body;
		RigidBody* B = contact.b->body;

		FLOAT3 posA = A ? A->predPos : FLOAT3{};
		FLOAT3 posB = B ? B->predPos : FLOAT3{};

		FLOAT3 ra = contact.point - (A ? A->predPos : FLOAT3{});
		FLOAT3 rb = contact.point - (B ? B->predPos : FLOAT3{});

		float wA = generalizedInvMass(A, ra, contact.normal);
		float wB = generalizedInvMass(B, rb, contact.normal);
		float wSum = wA + wB;
		if (wSum == 0) continue;

		//PBD: distance constraint C = l-l0 = l;
		float C = contact.penetration;
		if (C <= 0) {  //if (C <= 0.01) {  //
			continue;
		}

		//if (C <= 0.01f ) continue;
		// XPBD: α = compliance / dt²
		float alpha = compliance * inv_dt2;
		//float dLambda = (C + alpha * contact.lambda) / (wSum + alpha);
		float dLambda = (C) / (wSum + alpha);
		contact.lambda = dLambda;

		//PBD: impulse direction:
		FLOAT3 N = contact.normal; // contact normal

		FLOAT3 corr = dLambda * N;
		//apply correction to predicted positions:
		if (A) A->predPos += corr * wA;
		if (B) B->predPos -= corr * wB;

		auto applyRot = [&](RigidBody* rb, const FLOAT3& r, const FLOAT3& dir, float lambda, float sign)
			{
				if (!rb || !rb->simulatePhysics || !rb->simulateRotation) return;

				if (LengthSq(Vector3Cross(r, dir * sign)) < 1e-4f)
				{
					//std::cout << "\t Zero angular correction?" << '\n';
					//return;
				}

				// dω = invI (r × Δp)
				FLOAT3 dOmega = rb->invWorldInertia * Vector3Cross(r, dir);
				XMVECTOR q = rb->predRot;
				XMVECTOR dq = XMQuaternionMultiply(q,
					XMVectorSet(dOmega.x(), dOmega.y(), dOmega.z(), 0.f));
				dq = XMVectorScale(dq,  0.5f * dLambda);
				rb->predRot = XMQuaternionNormalize(q + sign * dq);
			};

		if (A && A->simulateRotation)
			applyRot(A, ra, N, dLambda, +1.f);

		if (B && B->simulateRotation)
			applyRot(B, rb, N, dLambda, -1.f);


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

		//rb->linearVelocity *= 0.999f;  //debug damping;
		//if (LengthSq(rb->linearVelocity) < 1e-3f)  
		//	rb->linearVelocity = FLOAT3{}; //reset to zero if too small


		// PostPBD()
		if (!rb->simulateRotation) continue; //skip if not enabled
		rb->rotation = rb->predRot;

		XMVECTOR dq = XMQuaternionMultiply(rb->predRot,
			XMQuaternionInverse(rb->prevRot));

		FLOAT3 v = { dq.m128_f32[0], dq.m128_f32[1], dq.m128_f32[2] };
		float  w = dq.m128_f32[3];
		if (w < 0.f) v = -v;
		rb->angularVelocity = (2.f / delta) * v;

		//if (LengthSq(rb->angularVelocity) < 0.1f) {
		//	rb->angularVelocity = FLOAT3{};
		//}

		//std::cout << "rb: " << rb->debugName
		//	<< ",delta q w: " << w
		//	<< ",delta q xyz: " << v.x() << "," << v.y() << "," << v.z() << '\n';
		//<< ", angVel: " << rb->angularVelocity.x() << ", "
		//<< rb->angularVelocity.y() << ", "
		//<< rb->angularVelocity.z() << std::endl;
	    //angular damping:
	    //rb->angularVelocity *= 0.999f; //test damping;
	}

}

void PhysicsScene::VelocityPass(float delta)
{
	for (Contact& c : m_contacts)
	{
		RigidBody* A = c.a->body;
		RigidBody* B = c.b->body;

		FLOAT3 ra = c.point - (A ? A->predPos : FLOAT3{});
		FLOAT3 rb = c.point - (B ? B->predPos : FLOAT3{});


		//--------------------------------- 
		auto w = [&](RigidBody* r, const FLOAT3& rVec)->float
			{
				if (!r || !r->simulatePhysics) return 0.f;
				FLOAT3 cr = Vector3Cross(rVec, c.normal);
				return r->invMass + Dot(cr, r->invWorldInertia * cr);
			};
		float wA = w(A, ra);
		float wB = w(B, rb);
		float invMassN = wA + wB;
		if (invMassN == 0.f) continue;


		//--------------------------------- 
		FLOAT3 vA = A ? (A->linearVelocity + Vector3Cross(A->angularVelocity, ra)) : FLOAT3{};
		FLOAT3 vB = B ? (B->linearVelocity + Vector3Cross(B->angularVelocity, rb)) : FLOAT3{};
		FLOAT3 vRel = vA - vB;

		float vn = Dot(vRel, c.normal);
		if (vn > 0.f) continue; 

		//restore the jn by lambda:   dx/dt = mimic v
		float jn =  c.lambda / delta ; 

		//---------------------------------
		//restitution  
		float eA = A ? A->material.restitution : 0.f;
		float eB = B ? B->material.restitution : 0.f;
		float e = std::min(eA, eB);             //avg/max
		 jn = (1+ e) * jn;
		 jn /= invMassN; 



		//---------------------------------
		// 4) friction impulse（Coulomb）
		FLOAT3 vT = vRel - vn * c.normal; 
		float vTLength = Length(vT);
		if (vTLength < 1e-6f) vT = FLOAT3{};
		else {
			vT = Normalize(vT); 
		} 

		auto invEffMass = [&](RigidBody* r, const FLOAT3& rVec, const FLOAT3& dir)->float
			{
				if (!r || !r->simulatePhysics) return 0.f;
				FLOAT3 cr = Vector3Cross(rVec, dir);
				return r->invMass + Dot(cr, r->invWorldInertia * cr);
			};
		float invMassT = invEffMass(A, ra, vT) + invEffMass(B, rb, vT);


		float desiredJt = -vTLength / invMassT;

		//hardcode friction coefficients:
		float muA_s = A ? A->material.friction : 0.f; // static friction
		float muB_s = B ? B->material.friction : 0.f; 
		float muA_k = 0.5f * muA_s; // kinetic friction
		float muB_k = 0.5f * muB_s;  

		float mu_s = std::sqrt(muA_s * muB_s);  
		float mu_k = std::sqrt(muA_k * muB_k);

		float maxStatic = mu_s * jn;
		float jt = 0.f;

		// | jt | ≤ μ | jn |
		jt = desiredJt;
		//if (std::fabs(desiredJt) < maxStatic)
		//{ 
		//	
		//}
		//else
		//{
		//	//μk·jn
		//	jt = -mu_k * jn * (desiredJt > 0 ? 1.f : -1.f);
		//}
		
 

		//--------------------------------- 
		FLOAT3 impulse = jn * c.normal + jt * vT; // impulse vector

		auto applyImpulse = [&](RigidBody* r,
			const FLOAT3& rVec,
			const FLOAT3& imp, float sign)
			{
				if (!r || !r->simulatePhysics) return;
				r->linearVelocity += sign * imp * r->invMass;
				r->angularVelocity += sign * (r->invWorldInertia *
					Vector3Cross(rVec, imp));
			};

		applyImpulse(A, ra, impulse, +1.f);
		applyImpulse(B, rb, impulse, -1.f);

		//std::cout << "Contact: " << c.a->body->debugName << " vs " << c.b->body->debugName
		//	<< ",lambda: " << c.lambda << ", "
		//	<< ",jn: " << jn << ", " 
		//	<< "，desiredJt: " << desiredJt
		//	<<", jt: " << jt * vT.x() << ", " << jt * vT.y() << ", " << jt * vT.z()
		//	<< ",impulse: " << impulse.x() << ", " << impulse.y() << ", " << impulse.z()
		//	<< '\n';
	}
}

void PhysicsScene::IntegratePosition(float delta)
{
	//for (auto& rb : m_bodies) {
	//	if (!rb->simulatePhysics) continue;
	//	//rb->position = rb->position + rb->linearVelocity * delta ;

	//	if (!rb->enableRotation) continue;
	//	//rotation by quaternion: 
	//	XMVECTOR q = rb->rotation; // 假设为 XMVECTOR 类型（x,y,z,w）

	//	XMVECTOR omegaQuat = XMVectorSet(
	//		rb->angularVelocity.x(),
	//		rb->angularVelocity.y(),
	//		rb->angularVelocity.z(),
	//		0.0f
	//	);

	//	// dq = 0.5 * omegaQuat * q
	//	XMVECTOR dq = XMQuaternionMultiply(omegaQuat, q);
	//	dq = XMVectorScale(dq, 0.5f);

	//	XMVECTOR updatedRot = XMVectorAdd(q, XMVectorScale(dq, delta));
	//	rb->rotation = XMQuaternionNormalize(updatedRot);

	//}
}

void PhysicsScene::PostSimulation()
{
	for (auto& rb : m_bodies) {
		rb->owner->SetWorldPosition(rb->position);

		//std::cout << "Position vs PredPos: " << rb->position.y() << " vs " << rb->predPos.y() << std::endl;

		if (!rb->simulateRotation) continue;
		rb->owner->SetWorldRotation(rb->rotation);
		//update world inertia:

	}
}