#include "PCH.h"
#include "PhysicsScene.h"

#include "Render/Renderer.h" 

#include "Collision.h"   

#include "PhysicsEvent.h"

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



void PhysicsScene::PreSimulation()
{
	//for (auto& [actor, rb] : m_bodies) {
	//	rb->position = rb->owner->position; 
	//}
	DebugDraw::ClearFrame();

	//new: consume the cmd buffer: 
	m_commandBuffer.SwapBuffers();
	m_commandBuffer.Execute();

	auto events = PhysicsEventQueue::Get().Drain();
	if (events.size() > 0)
		std::cout << "unhandled collision: " << events.size() << '\n';

	//add damping:
	for (auto& [actor, rb] : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->linearVelocity *= rb->linearDamping;
		rb->angularVelocity *= rb->angularDamping;
	}


	//DebugDraw::AddLine(Float3{ 0, 0, 0 }, Float3{ 5, 0, 0 }, Float4{ 1, 0, 0, 1 });
	//DebugDraw::AddLine(Float3{ 0, 0, 0 }, Float3{ 0, 5, 0 }, Float4{ 0, 1, 0, 1 });
	//DebugDraw::AddLine(Float3{ 0, 0, 0 }, Float3{ 0, 0, 5 }, Float4{ 0, 0, 1, 1 });
}

void PhysicsScene::ApplyExternalForce(float delta)
{
	for (auto& [actor, rb] : m_bodies) {
		if (!rb->simulatePhysics) continue;
		rb->ApplyForceRate(this->gravity * rb->mass * delta);
	}
}

void PhysicsScene::Integrate(float delta)
{
	for (auto& [actor, body] : m_bodies) {
		if (!body->simulatePhysics) continue;


		//std::cout << "integrate for rb: " << ToString(rb->force ) << '\n';
		body->linearVelocity = body->linearVelocity + body->force / body->mass * delta;
		body->force = Float3{};

		//cache the previous position:
		body->prevPos = body->position;

		//predicated position:
		body->predPos = body->position + body->linearVelocity * delta;

		if (!body->simulateRotation) {
			continue;
		}
		//new: consider torque: 

		//rb->angularVelocity = rb->angularVelocity + Inverse3x3(rb->worldInertia) * rb->torque * delta;
		//rb->torque = Float3{}; //reset torque 

		//
		body->prevRot = body->rotation;

		XMVECTOR omegaW = XMVectorSet(body->angularVelocity.x(), body->angularVelocity.y(), body->angularVelocity.z(), 0.0f);
		XMVECTOR dq = XMQuaternionMultiply(body->rotation, omegaW);
		dq = XMVectorScale(dq, 0.5f * delta);
		body->predRot = XMQuaternionNormalize(XMVectorAdd(dq, body->predRot));

		/*	std::cout << "integrate for rb : " << rb->debugName << '\n';
			std::cout << "dq: " << MMath::XMToString(dq) << '\n';*/

			//update pose;
			//XMMATRIX R_ = XMMatrixRotationQuaternion(rb->predRot);
		XMMATRIX R_ = XMMatrixRotationQuaternion(body->predRot);
		XMMATRIX invR_ = XMMatrixTranspose(R_);

		Float3x3 R;
		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };

		body->RotationMatrix = R;


		auto worldInertia = MatrixMultiply(MatrixMultiply(R, body->localInertia), Transpose(R));
		body->worldInertia = worldInertia;
		body->invWorldInertia = Inverse3x3(worldInertia);
	}
}




void PhysicsScene::DetectCollisions()
{
	m_contacts.clear();

	//std::cout << "Detecting collisions, colliders count: " << m_colliders.size() << std::endl;

	std::vector<WorldShapeProxy> ws;
	ws.reserve(m_colliders.size());
	for (auto& [actor, c] : m_colliders) {
		//new:
		if (!c->bEnabled) continue;

		WorldShapeProxy proxy = { MakeWorldShape(*c), c };
		ws.push_back(proxy);
	}


	std::vector<ColliderPair> pairs;
	m_broadPhase.ComputePairs(ws, pairs);


	//for (size_t i = 0; i < ws.size(); ++i)
	//	for (size_t j = i + 1; j < ws.size(); ++j)
	//	{
	//		auto& A = ws[i];
	//		auto& B = ws[j];

	for (auto& pr : pairs) { 
		WorldShapeProxy A{ MakeWorldShape(*pr.first), pr.first };
		WorldShapeProxy B{ MakeWorldShape(*pr.second), pr.second };

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
	constexpr float compliance = 0.000001f; // compliance factor 
	const float inv_dt2 = 1.f / (delta * delta); // 1 / dt²

	auto generalizedInvMass = [&](RigidBody* rb,
		const Float3& ri,
		const Float3& n)->float
		{
			if (!rb || !rb->simulatePhysics) return 0.f;

			if (rb->simulateRotation == false) {
				return rb->invMass;
			}
			Float3 cross = Vector3Cross(ri, n);
			Float3 tmp = rb->invWorldInertia * cross;
			return rb->invMass + Dot(cross, tmp);     // scalar
		};


	for (Contact& contact : m_contacts) {

		DebugDraw::AddCube(contact.point, 0.05f);
		//new:
		if (contact.a->bIsTrigger || contact.b->bIsTrigger) {
			FCollisionEvent event = {
				.a_ID = contact.a->actorId,
				.b_ID = contact.b->actorId,
				.contact = contact,
			};
			PhysicsEventQueue::Get().Push(event);
			continue;
		}

		RigidBody* A = contact.a->body;
		RigidBody* B = contact.b->body;

		//new: the architecture now assumes valid rigidbody;
		assert(A != nullptr && B != nullptr);

		Float3 posA = A->predPos;
		Float3 posB = B->predPos;

		Float3 ra = contact.point - (A->predPos);
		Float3 rb = contact.point - (B->predPos);

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
		// XPBD: α = compliance / dt|2
		float alpha = compliance * inv_dt2;
		float dLambda = (C) / (wSum + alpha);
		contact.lambda = dLambda;

		// XPBD solve with warm start 
		//ContactKey key(contact.a, contact.b);
		//float cachedLambda = m_lambdaCache[key];
		//float dLambda = (C + alpha * cachedLambda) / (wSum + alpha); 
		//float newLambda = cachedLambda + dLambda;  
		//m_lambdaCache[key] = newLambda; // update the cache with the new lambda 
		//contact.lambda = dLambda; // store the lambda for this contact

		//PBD: impulse direction:
		Float3 N = contact.normal; // contact normal

		Float3 corr = dLambda * N;
		//apply correction to predicted positions:
		if (A) A->predPos += corr * wA;
		if (B) B->predPos -= corr * wB;

		auto applyRot = [&](RigidBody* rb, const Float3& r, const Float3& dir, float lambda, float sign)
			{
				if (!rb || !rb->simulatePhysics || !rb->simulateRotation) return;

				if (LengthSq(Vector3Cross(r, dir * sign)) < 1e-4f)
				{
					//std::cout << "\t Zero angular correction?" << '\n';
					//return;
				}

				// dω = invI (r × Δp)
				Float3 dOmega = rb->invWorldInertia * Vector3Cross(r, dir);
				XMVECTOR q = rb->predRot;
				XMVECTOR dq = XMQuaternionMultiply(q,
					XMVectorSet(dOmega.x(), dOmega.y(), dOmega.z(), 0.f));
				dq = XMVectorScale(dq, 0.5f * dLambda);
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
	for (auto& [actor, body] : m_bodies) {
		if (!body->simulatePhysics) continue;

		body->position = body->predPos; //update position to predicted position 
		body->linearVelocity = (body->predPos - body->prevPos) / delta;

		//rb->linearVelocity *= 0.999f;  //debug damping;
		//if (LengthSq(rb->linearVelocity) < 1e-3f)  
		//	rb->linearVelocity = Float3{}; //reset to zero if too small 

		// PostPBD()
		if (!body->simulateRotation) continue; //skip if not enabled
		body->rotation = body->predRot;

		//XMVECTOR dq = XMQuaternionMultiply(rb->predRot, XMQuaternionInverse(rb->prevRot));
		XMVECTOR dq = XMQuaternionMultiply(XMQuaternionInverse(body->prevRot), body->predRot);

		Float3 v = { dq.m128_f32[0], dq.m128_f32[1], dq.m128_f32[2] };
		float  w = dq.m128_f32[3];
		if (w < 0.f) v = -v;
		if (body->bFastStable && LengthSq(v) < 1e-10f / body->mass) {
			v = Float3{}; 
		}
		body->angularVelocity = (2.f / delta) * v;

		//std::cout << "post pbd for rb: " << rb->debugName << '\n';
		//std::cout <<  " ang vel: " << ToString(rb->angularVelocity) << '\n';

		XMMATRIX R_ = XMMatrixRotationQuaternion(body->rotation);
		Float3x3 R;
		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };

		body->RotationMatrix = R;

		body->worldInertia = MatrixMultiply(MatrixMultiply(R, body->localInertia), Transpose(R));
		body->invWorldInertia = Inverse3x3(body->worldInertia);
		//if (LengthSq(rb->angularVelocity) < 0.1f) {
		//	rb->angularVelocity = Float3{};
		//} 
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

		//Float3 ra = c.point - (A ? A->predPos : Float3{});
		//Float3 rb = c.point - (B ? B->predPos : Float3{}); 

		Float3 ra = c.point - A->position;
		Float3 rb = c.point - B->position;

		//--------------------------------- 

		//auto invEffMass = [&](RigidBody* r, const Float3& rVec, const Float3& dir)->float
		//	{
		//		if (!r || !r->simulatePhysics) return 0.f;
		//		Float3 cr = Vector3Cross(rVec, dir);
		//		return r->invMass + Dot(cr, r->invWorldInertia * cr);
		//	};
		//float invMassT = invEffMass(A, ra, vT) + invEffMass(B, rb, vT);


		//--------------------------------- 
		Float3 vA = A->linearVelocity + Vector3Cross(A->angularVelocity, ra);
		Float3 vB = B->linearVelocity + Vector3Cross(B->angularVelocity, rb);
		Float3 vRel = vA - vB;
		DebugDraw::AddRay(c.point, vRel, Color::Cyan);

		float vn = Dot(vRel, c.normal);
		//already separating:
		if (vn > 0.0) continue;

		Float3 vT = vRel - vn * c.normal;

		auto invGenMassT = [&](RigidBody* r, const Float3& rVec, const Float3& dir)->float
			{
				if (!r->simulatePhysics) return 0.f;
				Float3 cr = Vector3Cross(rVec, dir);
				return r->invMass + Dot(cr, r->invWorldInertia * cr);
			};
		float invMassSum = invGenMassT(A, ra, vT) + invGenMassT(B, rb, vT);
		if (invMassSum == 0.f) {
			//std::cerr << "two unsimulated objects?" << '\n';
			continue;
		}

		//DebugRay::AddRay(A->position, ra, Color::Pink);
		//DebugDraw::Get().AddRay( A->position, ra, Color::Pink);
		//DebugDraw::Get().AddRay(c.point, vA, Color::Pink);

		////DebugDraw::Get().AddRay( B->position, rb, Color::Pink);
		//DebugDraw::Get().AddRay(c.point, vB, Color::Pink); 

		/*DebugRay::AddRay(c.point, vRel, Color::Orange);*/
		//DebugDraw::Get().AddRay(c.point, vRel, Color::Orange);
		//std::cout << "rel vel:" << ToString(vRel) << '\n'; 

		//restore the jn by lambda:  f = lambuda * normal / dt ^2;
		float jn = c.lambda / delta;
		Float3 mimicF = jn * c.normal / delta;

		DebugDraw::AddRay(c.point, mimicF, Color::Cyan);
		//DebugDraw::Get().AddRay(c.point, mimicF, Color::Cyan);


		//---------------------------------
		//restitution  

		float eA = A->material.restitution;
		float eB = B->material.restitution;
		float e = std::min(eA, eB);             //avg/max
		jn = (1 + e) * jn;

		//std::cout << "jn before: " << jn << '\n';
		//jn /= invMassSum; // scale by effective mass


		//---------------------------------
		// friction cone
		// | jt | ≤ μ | jn |

		float vTMag = Length(vT);
		if (vTMag < 1e-6f) vT = Float3{};
		else {
			vT = Normalize(vT);
		}


		float desiredJt = vTMag / invMassSum;

		//stick-slip
		// kinetic friction is hardcoded 0.8 of static;
		float muA_s = A->material.friction;
		float muB_s = B->material.friction;
		float muA_k = 0.8f * muA_s;
		float muB_k = 0.8f * muB_s;

		//float mu_s = std::sqrt(muA_s * muB_s);
		//float mu_k = std::sqrt(muA_k * muB_k);
		float mu_s = std::max(muA_s, muB_s);
		float mu_k = std::max(muA_k, muB_k);

		float maxStatic = mu_s * jn;
		float jt = 0.f;

		//jt = std::clamp(desiredJt, -maxStatic, maxStatic); // clamp to static friction limit 
		if (desiredJt < maxStatic)
		{
			jt = desiredJt;
			//std::cout << "no slip detected";
		}
		else
		{ 
			jt = mu_k * jn;
			//std::cout << "slip detected, mu_k: " << mu_k << " jn: " << jn << '\n';
		}

		//------------------------------ 
		// composed impulse 
		//Float3 impulse = jn * c.normal + -jt * vT; // impulse vector
		Float3 impulse = -jt * vT; // impulse vector
		DebugDraw::AddRay(c.point, vT, Color::Brown);
		//DebugDraw::AddRay(c.point, impulse / delta, Color::Brown);
		//std::cout << "Impulse: " << ToString(impulse) << '\n';

		auto applyImpulse = [&](RigidBody* r,
			const Float3& rVec,
			const Float3& imp, float sign)
			{
				if (!r->simulatePhysics) return;
				r->linearVelocity += sign * imp * r->invMass;

				if (!r->simulateRotation) return;
				r->angularVelocity += sign * (r->invWorldInertia *
					Vector3Cross(rVec, imp));
			};

		applyImpulse(A, ra, impulse, +1.f);
		applyImpulse(B, rb, impulse, -1.f);

	}
}


void PhysicsScene::PostSimulation()
{
	//for (auto& [actor, rb] : m_bodies) {
	//	 
	//	//::cout << "draw for rb:" << rb->debugName << '\n'; 
	//	if (!rb->simulatePhysics) continue;
	//	rb->owner->SetWorldPosition(rb->position); 

	//	DebugDraw::Get().AddRay(rb->position, rb->linearVelocity, Color::Purple); 

	//	if (!rb->simulateRotation) continue;
	//	rb->owner->SetWorldRotation(rb->rotation);  

	//	DebugDraw::Get().AddRay(rb->position, rb->angularVelocity, Color::Yellow);

	//	//XMVECTOR axis;
	//	//float angle;
	//	//XMQuaternionToAxisAngle(&axis, &angle, rb->rotation);
	//	//XMFLOAT3 axisVec;
	//	//XMStoreFloat3(&axisVec, axis); 

	//	//DebugDraw::Get().AddRay(rb->position, { axisVec.x,axisVec.y,axisVec.z }, Color::Cyan);

	//}

	//write to transform buffer:
	auto& writeBuffer = m_transformBuffer.GetWriteBuffer();
	//std::cout << "physics buffer size: " << writeBuffer.size() << '\n';
	for (auto& [actor, rb] : m_bodies) {

		auto& transform = writeBuffer[actor];
		transform.position = rb->position;
		transform.rotation = rb->rotation;

		//DebugDraw::AddRay(rb->position, rb->linearVelocity, Color::Purple);
		//DebugDraw::AddRay(rb->position, rb->angularVelocity, Color::Yellow);
	}

	m_transformBuffer.SwapBuffers();
}

void PhysicsScene::SetPosition(ActorId handle, const Float3& position)
{
	m_commandBuffer.Enqueue([=]() {
		auto it = m_bodies.find(handle);
		if (it != m_bodies.end()) {
			it->second->SetPosition(position);
		}
		else {
			//std::cerr << "RigidBody with handle " << handle << " not found!" << std::endl;
		}
		});

}

void PhysicsScene::SetRotation(ActorId handle, const DirectX::XMVECTOR& rotation)
{
	m_commandBuffer.Enqueue([=]() {
		auto it = m_bodies.find(handle);
		if (it != m_bodies.end()) {
			it->second->SetRotation(rotation);
		}
		else {
			//std::cerr << "physics: RigidBody with handle: " << handle << " not found!" << std::endl;
		}
		});
}

void PhysicsScene::AddRigidBody(RigidBody* rb, ActorId owner,
	const Float3& position,
	const DirectX::XMVECTOR& rotation
)
{
	m_commandBuffer.Enqueue([=]() {
		m_bodies[owner] = rb;
		//m_transformBuffer.Write(owner, PhysicsTransform{}); 
		//bug fix: don't forget clear the state;
		});
	//this->m_bodies.push_back(rb); 
}

void PhysicsScene::AddCollider(Collider* collider, ActorId owner)
{
	m_commandBuffer.Enqueue([=]() {
		collider->actorId = owner;
		m_colliders[owner] = collider;
		});  
}

void PhysicsScene::RemoveRigidBody(ActorId owner)
{
	m_commandBuffer.Enqueue([=]() {
		if (m_bodies.contains(owner))
			m_bodies.erase(owner); 

		m_transformBuffer.MarkToRemove(owner);

		});

	//assert(m_bodies.contains(owner));

}

void PhysicsScene::RemoveCollider(ActorId owner)
{
	m_commandBuffer.Enqueue([=]() {
		if (m_colliders.contains(owner))
			m_colliders.erase(owner);
		});
	//assert(m_colliders.contains(owner)); 
}

/*
RigidBody::RigidBody(StaticMeshActorProxy* owner, ShapeType type)
	: owner(owner), type(type)
{
	position = owner->position;
	predPos = owner->position;
	prevPos = owner->position;

	rotation = owner->rotation;
	predRot = owner->rotation;
	prevRot = owner->rotation;


	localInertia = MakeInertiaTensor(type, mass);
	//std::cout << "RigidBody created: " << typeid(type).name() << std::endl;
	DirectX::XMMATRIX R_ = DirectX::XMMatrixRotationQuaternion(rotation);
	Float3x3 R;
	R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
	R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
	R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };
	RotationMatrix = R;

	worldInertia = MatrixMultiply(MatrixMultiply(R, localInertia), Transpose(R));
	invWorldInertia = Inverse3x3(worldInertia);

}
*/

RigidBody::RigidBody()
{
	//localInertia = MakeInertiaTensor(type, mass);
}

 

void BroadPhase::ComputePairs(std::vector<WorldShapeProxy>& ws, std::vector<ColliderPair>& out)
{
	out.clear();
	out.reserve(ws.size() * ws.size());

	//std::vector<AABB> aabbs;
	//aabbs.reserve(inWS.size()); 
 
	for (size_t i = 0; i < ws.size(); ++i)
	  for (size_t j = i + 1; j < ws.size(); ++j)
	{
		  auto& AABB0 = ws[i].owner->aabb;
		  auto& AABB1 = ws[j].owner->aabb;
  
		  float pad = 0.02f;  
		  AABB fat0 = ExpandFatAABB(AABB0, pad);
		  AABB fat1 = ExpandFatAABB(AABB1, pad);

		  Contact c;
		  if (Collide(fat0, fat1, c)) {
			  //std::cout << "BroadPhase: AABB overlap detected: " << i << " vs " << j << '\n';
			  out.emplace_back(ws[i].owner, ws[j].owner);
		  }
	}
 
}
