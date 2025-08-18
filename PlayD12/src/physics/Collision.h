#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h" 
#include "PhysicsScene.h"

//std::min, max, clamp..
#include <algorithm>

#include "CollisionUtils.h"
/*
* convention:
* the contact normal is :  b to a;
* 
* depth value is stored as positive;

*/

 
struct WorldShapeProxy
{
    WorldShape shape;
    Collider* owner;
};

WorldShape MakeWorldShape(Collider& c)
{
    return std::visit([&](auto const& s) -> WorldShape {

        using Shape = std::decay_t<decltype(s)>;

        if constexpr (std::is_same_v<Shape, Sphere>)
        { 
            assert(s.radius > 1e-6f);
            Float3 center = c.body->predPos; 
			SphereWS sphereWS{ center, s.radius };

			c.aabb = MakeAABB(center, s.radius);
			//DrawDebugSphere(sphereWS);  
			return sphereWS;  
        }
        else if constexpr (std::is_same_v<Shape, Box>)
        {  
    //        if (XMQuaternionEqual(c.body->rotation, XMQuaternionIdentity())) {
    //            //if the body is not rotating, we can use AABB
    //            const Float3 center = c.body->predPos;
				//AABB aabb{ center - s.halfExtents, center + s.halfExtents };
				//DrawDebugOBB(MakeOBB(aabb)); 
				//return aabb; 
    //        }

            const Float3 center = c.body->predPos;
            //return as OBB:
            Float3x3 R = c.body->RotationMatrix;
            OBB obb;
            obb.center = center;
            obb.halfExtents = s.halfExtents;
            obb.axis[0] = R[0]; //right
            obb.axis[1] = R[1]; //up
            obb.axis[2] = R[2]; //forward 

            //new:
			c.aabb = MakeAABB(obb.center, R, obb.halfExtents); 
			DrawDebugOBB(obb); //draw debug OBB

            return obb;
        }
        else if constexpr (std::is_same_v<Shape, Plane>)
        {
            //const Float3 n = Float3{ 0, 1, 0 }; //assuming the plane is horizontal for now  
            //const float  d = -Dot(n, c.body->position);
            //const Float3 center = c.body->position; 
            //return PlaneWS{ n, d , s.width, s.height, center };

			//new: we are using box for planes now
    //        if (XMQuaternionEqual(c.body->rotation, XMQuaternionIdentity())) {
    //      
				////if the body is not rotating, we can use AABB
				//const Float3 center = c.body->predPos;
				//AABB aabb{ center - Float3{ s.width * 0.5f, 1e-2, s.height * 0.5f },
				//		   center + Float3{ s.width * 0.5f, 1e-2, s.height * 0.5f } };

				//DrawDebugOBB(MakeOBB(aabb)); //draw debug AABB
				//return aabb; //return as AABB
    //        }

            //turn plane to OBB:
            assert(!MMath::NearZero(c.body->RotationMatrix));
			const Float3 center = c.body->predPos;
			Float3x3 R = c.body->RotationMatrix; 
			OBB obb;
			obb.center = center;
			obb.halfExtents = Float3{ s.width * 0.5f, 0.1f, s.height * 0.5f };  
			obb.axis[0] = R[0]; //right
			obb.axis[1] = R[1]; //up
			obb.axis[2] = R[2]; //forward 

			c.aabb = MakeAABB(obb.center, R, obb.halfExtents); //update AABB
			DrawDebugOBB(obb); 

			return obb; 
        } 

        else if constexpr (std::is_same_v<Shape, Capsule>)
        {
            const Float3 p0 = (c.body ? c.body->position : Float3{});
            const Float3 p1 = p0 + Float3{ 0, s.height, 0 }; //assuming vertical capsule for now
            return CapsuleWS{ p0, p1, s.radius };
        }

		else if constexpr (std::is_same_v<Shape, EmptyShape>)
        {
			return EmptyWS{}; //no shape, empty collider
		}
		else
        {
            static_assert(always_false<Shape>, "Shape not supported");
        }

        }, c.type);
} 

//fallback for unsupported shape combinations
//bug fix:  constant qualifier mismatch
    template<class A, class B> [[nodiscard]]
    inline bool Collide(const A& a, const B& b, Contact& out) {
        std::cout << "Unsupported collision: "
            << typeid(A).name() << " vs " << typeid(B).name() << std::endl;
        return false;
    }
    

    [[nodiscard]]
    bool Collide(const AABB& a, const AABB& b, Contact& out)
    {
        //std::cout << "Collide AABB with AABB" << std::endl;

        //run a interval overlap
        //if (a.max.x() < b.min.x() || a.min.x() > b.max.x()) return false;
        //if (a.max.y() < b.min.y() || a.min.y() > b.max.y()) return false;
        //if (a.max.z() < b.min.z() || a.min.z() > b.max.z()) return false;

        //---------------------
		//the overlap lenth must be positive if overlapping;
        float dx, dy, dz;
        if (!IntervalOverlap({ a.min.x(), a.max.x() }, { b.min.x(), b.max.x() }, dx)) return false;
        if (!IntervalOverlap({ a.min.y(), a.max.y() }, { b.min.y(), b.max.y() }, dy)) return false;
        if (!IntervalOverlap({ a.min.z(), a.max.z() }, { b.min.z(), b.max.z() }, dz)) return false;

        //dx = std::min(a.max.x() - b.min.x(), b.max.x() - a.min.x());
        //dy = std::min(a.max.y() - b.min.y(), b.max.y() - a.min.y());
        //dz = std::min(a.max.z() - b.min.z(), b.max.z() - a.min.z());
         
        Float3 normal;
        float penetration; 
        //decide on minimum overlap and normal dir.
        //pass the overlap test means the values are positive.

        if (dx < dy && dx < dz) {
            normal = (a.min.x() < b.min.x()) ? Float3{ -1, 0, 0 } : Float3{ 1, 0, 0 };
            penetration = dx;
        }
        else if (dy < dz) {
            normal = (a.min.y() < b.min.y()) ? Float3{ 0, -1, 0 } : Float3{ 0, 1, 0 };
            penetration = dy;
            //std::cout << "penetration Y: " << penetration << '\n';
        }
        else {
            normal = (a.min.z() < b.min.z()) ? Float3{ 0, 0, -1 } : Float3{ 0, 0, 1 };
            penetration = dz;
            //std::cout << "penetration Z: " << penetration << '\n';
        }

		//point takes middle of interval intersection:
        Float3 point = (Float3{
            std::max(a.min.x(), b.min.x()),
            std::max(a.min.y(), b.min.y()),
            std::max(a.min.z(), b.min.z())
            } + Float3{
                std::min(a.max.x(), b.max.x()),
                std::min(a.max.y(), b.max.y()),
                std::min(a.max.z(), b.max.z())
            }) * 0.5f;

        out.normal = normal;
        out.point = point;
        out.penetration = penetration; 

        return true;
    }

    [[nodiscard]]
    bool Collide(const SphereWS& a, const SphereWS& b, Contact& out)
    {
        //std::cout << "Collide Sphere with Sphere" << std::endl;

        Float3 a2b = b.center - a.center;
        float  distSq = LengthSq(a2b);
        float  r_sum = a.radius + b.radius;

        if (distSq >= r_sum * r_sum)
            return false;

        float dist = std::sqrt(distSq);
        float penetration = r_sum - dist;

        //defensive check.
        //bug fix : if the direction is negative; the spheres will get entangled;
        Float3 normal = Normalize(-1.0f * a2b); 

        //take the middle:
        Float3 surfaceA = a.center - normal * a.radius;
        Float3 surfaceB = b.center + normal * b.radius;
        Float3 contactPoint = (surfaceA + surfaceB) * 0.5f;

        out.normal = normal;
        out.point = contactPoint;
        out.penetration = penetration;
        return true;
    }
     

    [[nodiscard]]
    bool Collide(const SphereWS& s, const AABB& box, Contact& out)
    {
        //std::cout << "Collide Sphere with AABB" << std::endl;

		//reduce to closed point test:
        Float3 closestPoint = ClosestPoint(box, s.center);

        Float3 offset = s.center - closestPoint;
        float distSq = LengthSq(offset); 
        if (distSq > s.radius * s.radius) return false;

		//dist degeneracy is excluded by sphere radius check;
        float dist = std::sqrt(distSq); 

        out.normal = offset / dist;
        out.point = closestPoint;
        out.penetration = s.radius - dist;
 
        return true;
    }


    [[nodiscard]]
    bool Collide(const AABB& box, const SphereWS& s, Contact& out)
    {
        //std::cout << "Collide AABB with Sphere" << std::endl;

        if (!Collide(s, box, out)) return false;
        out.normal = -out.normal;
        return true;
    } 


    [[nodiscard]]
    bool Collide(const SphereWS& s, const OBB& box, Contact& out)
    {
        //std::cout << "Collide Sphere with OBB" << std::endl;
        //project the sphere onto the OBB's axes
		Float3 closestPoint = ClosestPoint(box, s.center); 
        //DebugDraw::AddCube(closestPoint, 0.05f, Color::Green);

        Float3 offset = s.center - closestPoint;
        float distSq = LengthSq(offset);
        if (distSq > s.radius * s.radius) 
            return false;

        float dist = std::sqrt(distSq); 
        out.normal = offset / dist;
        out.point = closestPoint;
        out.penetration = s.radius - dist;
        return true; 
    }

    [[nodiscard]]
    bool Collide(const OBB& box, const SphereWS& s, Contact& out)
    {
        //std::cout << "Collide OBB with Sphere" << std::endl;
        if (!Collide(s, box, out)) return false;
        out.normal = -out.normal;  
        return true;
    }
     
    // Returns an extreme vertex on obb in direction (support mapping).
    static Float3 SupportPoint(const OBB& obb, const Float3& dir)
    {
        Float3 res = obb.center;
        res += obb.axis[0] * obb.halfExtents.x() * ((Dot(dir, obb.axis[0]) >= 0.f) ? 1.f : -1.f);
        res += obb.axis[1] * obb.halfExtents.y() * ((Dot(dir, obb.axis[1]) >= 0.f) ? 1.f : -1.f);
        res += obb.axis[2] * obb.halfExtents.z() * ((Dot(dir, obb.axis[2]) >= 0.f) ? 1.f : -1.f);
        return res;
    }

    enum class PenType { 
        Unknown,
		FaceA,
		FaceB,
        Edges,
    };


    struct Manifold { 
		std::array<float, 4> depths; 
        std::array<Float3, 4> points;
        int count = 0; 
    }; 
     
    struct PolyN { 
        std::array<Float3, 8> verts; 
        int count = 0; 
    }; 

 
    enum class EOBBAxis
	{ 
		X ,
		Y ,
		Z ,
	};

    struct FSignedAxis {
		EAxis axis;
		float sign;
    };
 

    inline PolyN OBBFace(const OBB& box, const FSignedAxis& signedAxis) {
    
        int axis = (int)signedAxis.axis;
		float sign = signedAxis.sign;
         
		Float3 n = box.axis[axis] * box.halfExtents[axis] * sign; // face normal
         
		Float3 u = box.axis[(axis + 1) % 3] * box.halfExtents[(axis + 1) % 3]; 
		Float3 v = box.axis[(axis + 2) % 3] * box.halfExtents[(axis + 2) % 3];
		// Create a polygon representing the face
         
        //face center
		Float3 c = box.center + n; // center of the face
        PolyN poly;
		poly.verts[0] = c - u - v; 
        poly.verts[1] = c + u - v;
        poly.verts[2] = c + u + v;
        poly.verts[3] = c - u + v;
        poly.count = 4;
        return poly; 

    }

	inline PlaneWS OBBFaceAsPlane(const OBB& box, const FSignedAxis& signedAxis) {

		int axis = (int)signedAxis.axis;
		float sign = signedAxis.sign;

		PlaneWS plane;
		plane.normal = box.axis[axis] * sign; // face normal
		plane.center = box.center + plane.normal * box.halfExtents[axis]; 

		plane.d = -Dot(plane.normal, plane.center); //dot(p, n) + d = 0

		plane.width = box.halfExtents[(axis + 1) % 3] * 2.0f;  
		plane.height = box.halfExtents[(axis + 2) % 3] * 2.0f;  

		plane.right = box.axis[(axis + 1) % 3]; // right vector
		plane.forward = box.axis[(axis + 2) % 3]; // forward vector 

		return plane;
	}

    struct PenMin {
        float depth = std::numeric_limits<float>::max();
        Float3  axisW = { 0,0,0 }; // world axis  
        PenType penType{};
        int faceAxis = 0;
		int edgeIdA = -1;  
		int edgeIdB = -1;  
    }; 

    bool OBBOverlap(const OBB& A, const OBB& B, PenMin& out)
    { 
        constexpr float kEps = 1e-6f;
        float kHysteresis = 1.0f;

        const Float3 axesA[3] = { A.axis[0], A.axis[1], A.axis[2] };
        const Float3 axesB[3] = { B.axis[0], B.axis[1], B.axis[2] };

        float   minOverlap = std::numeric_limits<float>::max();
        Float3  bestAxis{};

        //flag state:
		PenType currType = PenType::Unknown;
		bool updated{ false };  

        // Helper lambda for the 15 SAT axes.
        auto TestAxis = [&](Float3 axis) -> bool
            {
                // degenerate case / almost parallel edges
                if (LengthSq(axis) < kEps) return true;
				axis = Normalize(axis); // normalize the axis

                auto intervalA = OBBProject(A, axis);
                auto intervalB = OBBProject(B, axis);
                float outOverlap = 0.0f;
                if (!IntervalOverlap(intervalA, intervalB, outOverlap)) {
                    return false;
                }

                //update candidate axis 
				updated = false; 
                if (outOverlap < minOverlap * kHysteresis) {
                    minOverlap = outOverlap;
                    bestAxis = axis;
                    out.penType = currType; 
					updated = true; 
                    //std::cout << "OBB vs OBB: update overlap: " << minOverlap << std::endl;
                }
                return true;
            };

        // 3 face normals of A
		currType = PenType::FaceA;  
        for (int i = 0; i < 3; ++i)
            if (!TestAxis(axesA[i])) return false;
            else {
                //std::cout << "OBB vs OBB: Face A test " << i << '\n';
				if (!updated) continue;  
				out.faceAxis = i; 
            }

        // 3 face normals of B
		currType = PenType::FaceB;  
        for (int i = 0; i < 3; ++i)
            if (!TestAxis(axesB[i])) 
                return false; 
			else {
				//std::cout << "OBB vs OBB: Face B test  " << '\n';
				if (!updated) continue;  
				out.faceAxis = i;  
			}

        // 9 edge–edge axes (cross products)
		currType = PenType::Edges;
		kHysteresis = 0.9f;  //edges update only when substantially better than best axis
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (!TestAxis(Vector3Cross(axesA[i], axesB[j])))
                    return false;
                else {
                    if (!updated) continue;
					out.edgeIdA = i;  
					out.edgeIdB = j; 
					//std::cout << "OBB vs OBB: Edge test " << '\n';
                }


        // handle contact
        // Ensure normal obeys Collision.h convention: "b to a"
        if (Dot(bestAxis, A.center - B.center) < 0.f)
            bestAxis = -bestAxis;

        out.axisW = bestAxis;
        out.depth = minOverlap; 

        return true;
    }

 
    Manifold BuildManifoldOBBFace(const OBB& A, const OBB& B, const PenMin& penMin)
    {
        Manifold manifold;

        //choose the reference and incident face based on penetration type
        const OBB& refOBB = (penMin.penType == PenType::FaceA) ? A : B;
        const OBB& incOBB = (penMin.penType == PenType::FaceA) ? B : A;

        //choose incident face of the incident OBB, 
        // by finding the axis "most non-aligned" with ref normal
        Float3 refNormal = penMin.axisW * (penMin.penType == PenType::FaceA ? -1.0f : +1.0f);
		DebugDraw::AddRay(refOBB.center, refNormal * 2.0f, Color::Yellow, Color::White); 
		//std::cout << "og normal: " << ToString(penMin.axisW) << std::endl;
		//std::cout << "refNormal: " << ToString(refNormal) << std::endl;

        FSignedAxis incAxis{};
        {
            float minDot = std::numeric_limits<float>::max();
            for (int i = 0; i < 3; ++i) {
                Float3 axis = incOBB.axis[i];
                float dot = Dot(axis, refNormal);
                if (dot < minDot) {
                    minDot = dot;
					incAxis.axis = static_cast<EAxis>(i);
					incAxis.sign = +1.0f; // positive sign
                }

                Float3 axis_neg = -incOBB.axis[i];
                float dot_neg = Dot(axis_neg, refNormal);
                if (dot_neg < minDot) {
                    minDot = dot_neg;
					incAxis.axis = static_cast<EAxis>(i);
					incAxis.sign = -1.0f; // negative sign 
                }
            }
			//std::cout << "incAxisIndex: " << incAxisIndex << " minDot: " << minDot << std::endl;
			//std::cout << "incAxis:" << ToString(incOBB.axis[std::abs(incAxisIndex)]) << std::endl;
            assert(minDot < 1e-6f); //expect pointing to the ref face;
        }


        FSignedAxis refAxis{};
        {
			float maxDot = std::numeric_limits<float>::lowest();
            for (int i = 0; i < 3; ++i) {
                Float3 axis = refOBB.axis[i];
                float dot = Dot(axis, refNormal);
                if (dot > maxDot) {
                    maxDot = dot;
					refAxis.axis = static_cast<EAxis>(i);
					refAxis.sign = +1.0f; 
                }

                Float3 axis_neg = -refOBB.axis[i];
                float dot_neg = Dot(axis_neg, refNormal);
                if (dot_neg > maxDot) {
                    maxDot = dot_neg;
					refAxis.axis = static_cast<EAxis>(i);
					refAxis.sign = -1.0f; 
                }
            }
			//std::cout << "refAxisIndex: " << refAxisIndex << " maxDot: " << maxDot << std::endl;
            assert(maxDot > 1e-6f); //expect pointing outward the ref face;
        } 

        PolyN incFace = OBBFace(incOBB, incAxis); // incident face polygon 
        PlaneWS refFacePlane = OBBFaceAsPlane(refOBB, refAxis); // reference face as plane
  
        for (int i = 0; i < incFace.count; ++i) {
			auto& vert = incFace.verts[i];
            float outDepth{ 0 };
             
            //DebugDraw::AddCube(vert, 0.1f, Color::Green);

            float depth = SignedDist(refFacePlane, vert);
			if (depth > 0.0f) {
				//outside the face, skip this vertex;
                //we might want to compute new vertex for better result?
				continue;
			}

            auto& clipV = ClipToPlane(refFacePlane, vert);
            manifold.points[manifold.count] = clipV; 
			manifold.depths[manifold.count] = std::abs(depth);  
			manifold.count++;
                 
            DebugDraw::AddCube(clipV, 0.1f, Color::Red);
            
        } 
		return manifold; 

	}
	

    Manifold BuildManifoldOBBEdges(const OBB& A, const OBB& B, const PenMin& penMin) {

        Manifold outM;

        //find the edge pair:
        //by "center difference"
		Float3 B2A = A.center - B.center;  
		DebugDraw::AddRay(B.center, B2A, Color::Cyan, Color::White); 

		auto& findEdge = [&](const OBB& box, const OBB& other, const int& axis, float sign) -> Segment { 

            Float3 u = box.axis[(axis + 1) % 3] * box.halfExtents[(axis + 1) % 3];
            Float3 v = box.axis[(axis + 2) % 3] * box.halfExtents[(axis + 2) % 3];
            
			auto closest = ClosestPoint(other, box.center);
			//DebugDraw::AddCube(closest, 0.05f, Color::Green);


			float signU = (Dot(closest - box.center, u) >= 0.f) ? 1.f : -1.f; // sign of u
			float signV = (Dot(closest - box.center, v) >= 0.f) ? 1.f : -1.f; // sign of v
 
			DebugDraw::AddLine(box.center, closest, Color::Green, Color::White);
			//auto supportU = SupportPoint(other, u);
			//auto supportV = SupportPoint(other, v);

			//DebugDraw::AddCube(supportU, 0.05f, Color::Green);
			//DebugDraw::AddCube(supportV, 0.05f, Color::Green);

			//float signU = (Dot(supportU, u) >= Dot(B2A, u)) ? 1.f : -1.f;
			//float signV = (Dot(supportV, v) >= Dot(B2A, v)) ? 1.f : -1.f;

			//float signU = (Dot(B2A, u) >= 0.f) ? 1.f : -1.f;  
			//float signV = (Dot(B2A, v) >= 0.f) ? 1.f : -1.f;

			//if (std::abs(Dot(B2A, u)) < 1e-4f) {
   //             signU = (Dot(SupportPoint(box, penMin.axisW) - box.center, u) >= 0.f) ? 1.f : -1.f;
			//}
			//if (std::abs(Dot(B2A, v)) < 1e-4f) {
			//	signV = (Dot(SupportPoint(box, penMin.axisW) - box.center, v) >= 0.f) ? 1.f : -1.f;
			//}

   //         signU *= sign;
   //         signV *= sign; 

			Float3 p0 = box.center + signU * u + signV * v - box.axis[axis] * box.halfExtents[axis]; 
			Float3 p1 = box.center + signU * u + signV * v + box.axis[axis] * box.halfExtents[axis];  

			return Segment{ p0, p1 }; 
			};

		Segment edgeA = findEdge(A, B, penMin.edgeIdA, -1); // reference edge
		Segment edgeB = findEdge(B, A, penMin.edgeIdB, 1); // incident edge

		DebugDraw::AddLine(edgeA.p0, edgeA.p1, Color::Red);
		DebugDraw::AddLine(edgeB.p0, edgeB.p1, Color::Blue);

		auto [pA, pB] = SegmentsClosest(edgeA, edgeB); 

		DebugDraw::AddCube(pA, 0.05f, Color::Red);
		DebugDraw::AddCube(pB, 0.05f, Color::Blue); 
        
        outM.points[outM.count] = (pA + pB) * 0.5f; 
		outM.depths[outM.count] = penMin.depth;   
        outM.count++;

		return outM; 
    }



    bool Collide(const OBB& A, const OBB& B, Contact& out)
    { 
		PenMin penMin;  
		if (!OBBOverlap(A, B, penMin)) {
			//std::cout << "OBB vs OBB: No overlap detected." << std::endl;
			return false; // no overlap
		} 

		Manifold manifold;
        // approximate contact point
        if (penMin.penType == PenType::FaceA || penMin.penType == PenType::FaceB) {
            // Face vs Face
			manifold = BuildManifoldOBBFace(A, B, penMin);
			//std::cout << "OBB vs OBB: Face-Face collision, overlap: " << penMin.depth << std::endl;
        } 
		else if (penMin.penType == PenType::Edges)
        { // Edges
			manifold = BuildManifoldOBBEdges(A, B, penMin);
			//std::cout << "OBB vs OBB: Edge-Edge collision, overlap: " << penMin.depth << std::endl;
		}
        else
        {
			std::cerr << "OBB vs OBB: Unknown penetration type!" << std::endl;
        }

		//simply average the contact points and depths
		Float3 avgPoint = Float3{ 0, 0, 0 };
		float avgDepth = 0.0f;

		for (int i = 0; i < manifold.count; ++i) {
			avgPoint += manifold.points[i] / static_cast<float>(manifold.count);
			avgDepth += manifold.depths[i] / static_cast<float>(manifold.count);
		} 
         
		out.point = avgPoint;
		out.normal = penMin.axisW; 
		out.penetration = std::abs(avgDepth); 

		//std::cout << "OBB vs OBB: avg depth: " << avgDepth << std::endl; 
 
        return true;  
    }
     


