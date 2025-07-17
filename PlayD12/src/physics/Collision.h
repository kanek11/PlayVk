#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h" 
#include "PhysicsScene.h"

//std::min, max, clamp..
#include <algorithm>

/*
* convention:
* the contact normal is :  b to a;

*/


struct AABB { Float3 min, max; };

struct OBB {
    Float3 center;
    Float3 axis[3];
    Float3 halfExtents;
};


struct SphereWS { Float3 center; float radius; };
struct CapsuleWS { Float3 p0, p1; float radius; };

// dot(p,n)+d = 0
struct PlaneWS {
    Float3 normal;
    float d;
    float width;
    float height;
    Float3 center;
    Float3 right = { 1, 0, 0 };
    Float3 forward = { 0, 0, 1 };
};

using WorldShape = std::variant<SphereWS, AABB, PlaneWS, OBB>;

struct WorldShapeProxy
{
    WorldShape shape;
    Collider* owner;
};

WorldShape MakeWorldShape(const Collider& c)
{
    return std::visit([&](auto const& s) -> WorldShape {

        using Shape = std::decay_t<decltype(s)>;

        if constexpr (std::is_same_v<Shape, Sphere>)
        {
            const Float3 center = (c.body ? c.body->predPos : Float3{});
            return SphereWS{ center, s.radius };
        }
        else if constexpr (std::is_same_v<Shape, Box>)
        {
            if (!c.body) {
                std::cerr << "Collider has no body" << std::endl;
                return OBB{};
            }

            if (!c.body->simulateRotation && XMQuaternionEqual(c.body->rotation, XMQuaternionIdentity())) {
                //if the body is not rotating, we can use AABB
                const Float3 center = c.body->predPos;
                return AABB{ center - s.halfExtents, center + s.halfExtents };
            }

            const Float3 center = c.body->predPos;
            //return as OBB:
            Float3x3 R = c.body->RotationMatrix;
            OBB obb;
            obb.center = center;
            obb.halfExtents = s.halfExtents;
            obb.axis[0] = R[0];
            obb.axis[1] = R[1];
            obb.axis[2] = R[2];
            return obb;
        }
        else if constexpr (std::is_same_v<Shape, Capsule>)
        {
            const Float3 p0 = (c.body ? c.body->position : Float3{});
            const Float3 p1 = p0 + Float3{ 0, s.height, 0 }; //assuming vertical capsule for now
            return CapsuleWS{ p0, p1, s.radius };
        }
        else if constexpr (std::is_same_v<Shape, Plane>)
        {
            const Float3 n = Float3{ 0, 1, 0 }; //assuming the plane is horizontal for now  
            const float  d = -Dot(n, (c.body ? c.body->position : Float3{}));
            const Float3 center = (c.body ? c.body->position : Float3{});

            return PlaneWS{ n, d , s.width, s.height, center };
        }
        else
        {
            static_assert(always_false<Shape>, "Shape not supported");
        }

        }, c.type);
}




//fallback for unsupported shape combinations
template<class A, class B> [[nodiscard]]
    inline bool Collide(const A& a, const B& b, Contact& out) {
        std::cout << "Unsupported collision: "
            << typeid(A).name() << " vs " << typeid(B).name() << std::endl;
        return false;
    }


    struct Interval {
        float min, max;
    };

    bool IntervalOverlap(Interval a, Interval b, float& overlap) {

        //if (a.max < b.min || a.min > b.max) return false;
        overlap = std::min(a.max, b.max) - std::max(a.min, b.min);

        if (overlap < 0.0f) {
            return false;
        }
        return true;
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

        //std::cout << "Collided AABB with AABB: " 
        //	<< "Penetration: " << out.penetration 
     //       << " Normal: " << out.normal.x() << ", " << out.normal.y() << ", " << out.normal.z() << '\n';

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
        Float3 normal;
        if (dist > 1e-6f)
            normal = Normalize(-1.0f * a2b);
        else
            normal = Float3{ 1, 0, 0 }; //meaningless fallback; todo:

        //take the middle:
        Float3 surfaceA = a.center - normal * a.radius;
        Float3 surfaceB = b.center + normal * b.radius;
        Float3 contactPoint = (surfaceA + surfaceB) * 0.5f;

        out.normal = normal;
        out.point = contactPoint;
        out.penetration = penetration;
        return true;
    }


    Float3 ClosedPoint(const AABB& box, const Float3& point) {
        Float3 closestPoint{
    std::clamp(point.x(), box.min.x(), box.max.x()),
    std::clamp(point.y(), box.min.y(), box.max.y()),
    std::clamp(point.z(), box.min.z(), box.max.z())
        };

        return closestPoint;
    }

    [[nodiscard]]
    bool Collide(const SphereWS& s, const AABB& box, Contact& out)
    {
        //std::cout << "Collide Sphere with AABB" << std::endl;

        Float3 closestPoint = ClosedPoint(box, s.center);

        Float3 offset = s.center - closestPoint;
        float distSq = LengthSq(offset);

        if (distSq > s.radius * s.radius) return false;

        float dist = std::sqrt(distSq);
        Float3 normal = dist > 0.0001f ? offset / dist : Float3{ 0, 1, 0 };
        Float3 point = closestPoint;

        out.normal = normal;
        out.point = point;
        out.penetration = s.radius - dist;

        //std::cout << "Collided Sphere with AABB: "
        //    << "Penetration: " << out.penetration
        //    << " Normal: " << out.normal.x() << ", " << out.normal.y() << ", " << out.normal.z() << '\n';

        return true;
    }


    [[nodiscard]]
    bool Collide(const AABB& box, const SphereWS& s, Contact& out)
    {
        //std::cout << "Collide AABB with Sphere" << std::endl;

        if (!Collide(s, box, out)) return false;
        out.normal = out.normal * -1.0f;
        return true;
    }



    [[nodiscard]]
    bool Collide(const SphereWS& s, const OBB& box, Contact& out)
    {
        //std::cout << "Collide Sphere with OBB" << std::endl;
        //project the sphere onto the OBB's axes
        Float3 closestPoint = box.center;
        for (int i = 0; i < 3; ++i) {
            closestPoint += box.axis[i] * std::clamp(Dot(s.center - box.center, box.axis[i]), -box.halfExtents[i], box.halfExtents[i]);
        }
        Float3 offset = s.center - closestPoint;
        float distSq = LengthSq(offset);
        if (distSq > s.radius * s.radius) return false;
        float dist = std::sqrt(distSq);
        Float3 normal = dist > 0.0001f ? offset / dist : Float3{ 0, 1, 0 };
        Float3 point = closestPoint;
        out.normal = normal;
        out.point = point;
        out.penetration = s.radius - dist;
        //std::cout << "Collided Sphere with OBB: "
        //	<< "Penetration: " << out.penetration
        //	<< " Normal: " << out.normal.x() << ", " << out.normal.y() << ", " << out.normal.z() << '\n';
        return true;

    }

    [[nodiscard]]
    bool Collide(const OBB& box, const SphereWS& s, Contact& out)
    {
        //std::cout << "Collide OBB with Sphere" << std::endl;
        if (!Collide(s, box, out)) return false;
        out.normal = -out.normal; //reverse the normal
        //std::swap(out.a, out.b); //swap the contact points
        return true;
    }



    [[nodiscard]]
    bool Collide(const AABB& box, const PlaneWS& plane, Contact& out)
    {
        const Float3 center = (box.min + box.max) * 0.5f;
        const Float3 extents = (box.max - box.min) * 0.5f;

        float r =
            extents.x() * std::abs(plane.normal.x()) +
            extents.y() * std::abs(plane.normal.y()) +
            extents.z() * std::abs(plane.normal.z());

        float d = Dot(plane.normal, center) + plane.d;

        if (std::abs(d) > r)
            return false;  // no intersection

        Float3 projected = center - plane.normal * d;
        Float3 localVec = projected - plane.center;

        //------------------------------
        float u = Dot(localVec, plane.right);
        float v = Dot(localVec, plane.forward);

        if (std::abs(u) > plane.width * 0.5f + extents.x() || std::abs(v) > plane.height * 0.5f + extents.z())
            return false;

        out.normal = (d < 0) ? plane.normal * -1.0f : plane.normal;
        out.penetration = r - std::abs(d);
        out.point = projected;


        return true;
    }

    [[nodiscard]]
    bool Collide(const PlaneWS& plane, const AABB& box, Contact& out)
    {
        //std::cout << "Collide Plane with AABB" << std::endl;
        if (!Collide(box, plane, out)) return false;

        out.normal = -out.normal;
        //std::swap(out.a, out.b);
        return true;
    }


    inline float SignedDist(const PlaneWS& plane, const Float3& point) {

        return Dot(plane.normal, point) + plane.d;
    }

    // Sphere vs Plane
    [[nodiscard]]
    bool Collide(const SphereWS& s, const PlaneWS& plane, Contact& out)
    {
        //std::cout << "Collide Sphere with Plane" << std::endl;

        //-----------------------
        float signedDist = SignedDist(plane, s.center);

        if (std::abs(signedDist) > s.radius)
            return false;


        //----------------------- 
        Float3 max = plane.center + 0.5f * plane.right * plane.width + 0.5f * plane.forward * plane.height;
        Float3 min = plane.center - 0.5f * plane.right * plane.width - 0.5f * plane.forward * plane.height;
        AABB mimicAABB = AABB{ min, max };

        if (!Collide(s, mimicAABB, out))
            return false;

        ////-----------------------
        //out.normal = (signedDist < 0.f) ? (plane.normal * -1.f) : plane.normal;
        //out.penetration = s.radius - std::abs(signedDist);
        //out.point = s.center - plane.normal * signedDist;
        return true;
    }

    // Plane vs Sphere
    [[nodiscard]]
    bool Collide(const PlaneWS& plane, const SphereWS& s, Contact& out)
    {
        //std::cout << "Collide Plane with Sphere" << std::endl;

        if (!Collide(s, plane, out))
            return false;

        out.normal *= -1.f;
        //std::swap(out.a, out.b);

        return true;
    }


    // Projects an oriented box onto an arbitrary axis and returns its radius on that axis.
    static float ProjectOBB(const OBB& obb, const Float3& n)
    {
        return obb.halfExtents.x() * std::abs(Dot(obb.axis[0], n)) +
            obb.halfExtents.y() * std::abs(Dot(obb.axis[1], n)) +
            obb.halfExtents.z() * std::abs(Dot(obb.axis[2], n));
    }

    inline Interval OBBProject(const OBB& obb, const Float3& axis) {
        //project the OBB onto a given axis

        float center = Dot(obb.center, axis);

        float radius = 0.0f;
        radius += obb.halfExtents.x() * std::abs(Dot(obb.axis[0], axis));
        radius += obb.halfExtents.y() * std::abs(Dot(obb.axis[1], axis));
        radius += obb.halfExtents.z() * std::abs(Dot(obb.axis[2], axis));

        return { center - radius, center + radius };
    }

    inline Interval AABBProject(const AABB& aabb, const Float3& axis) {
        //project the AABB onto a given axis
        Float3 aCenter = (aabb.min + aabb.max) * 0.5f;
        Float3 aExtent = (aabb.max - aabb.min) * 0.5f;

        float center = Dot(aCenter, axis);
        float radius = 0.0f;
        radius += aExtent.x() * std::abs(axis.x());
        radius += aExtent.y() * std::abs(axis.y());
        radius += aExtent.z() * std::abs(axis.z());
        return { center - radius, center + radius };

    }

    // Returns an extreme vertex on obb in direction "dir" (support mapping).
    static Float3 SupportVertex(const OBB& obb, const Float3& dir)
    {
        Float3 res = obb.center;
        res += obb.axis[0] * obb.halfExtents.x() * ((Dot(dir, obb.axis[0]) >= 0.f) ? 1.f : -1.f);
        res += obb.axis[1] * obb.halfExtents.y() * ((Dot(dir, obb.axis[1]) >= 0.f) ? 1.f : -1.f);
        res += obb.axis[2] * obb.halfExtents.z() * ((Dot(dir, obb.axis[2]) >= 0.f) ? 1.f : -1.f);
        return res;
    }


    bool Collide(const OBB& A, const OBB& B, Contact& out)
    {
        constexpr float kEps = 1e-6f;

        const Float3 A2B = B.center - A.center;

        const Float3 axesA[3] = { A.axis[0], A.axis[1], A.axis[2] };
        const Float3 axesB[3] = { B.axis[0], B.axis[1], B.axis[2] };

        float   minOverlap = FLT_MAX;
        Float3  bestAxis = { 0,0,0 };

        // Helper lambda for the 15 SAT axes.
        auto TestAxis = [&](const Float3& n) -> bool
            {
                if (LengthSq(n) < kEps) return true; // degenerate / parallel axis？

                const Float3 axis = Normalize(n);

                //float dist = std::abs(Dot(A2B, axis));
                //float ra = ProjectOBB(A, axis);
                //float rb = ProjectOBB(B, axis);
                //float overlap = ra + rb - dist;

                auto intervalA = OBBProject(A, axis);
                auto intervalB = OBBProject(B, axis);
                float overlap = 0.0f;
                if (!IntervalOverlap(intervalA, intervalB, overlap)) {
                    return false; // Separating axis found.
                }
                //if (overlap < 0.f) return false; // Separating axis – no collision.
                if (overlap < minOverlap) {
                    minOverlap = overlap;
                    bestAxis = axis;
                }
                return true;
            };

        // 3 face normals of A
        for (int i = 0; i < 3; ++i) if (!TestAxis(axesA[i])) return false;
        // 3 face normals of B
        for (int i = 0; i < 3; ++i) if (!TestAxis(axesB[i])) return false;
        // 9 edge–edge axes (cross products)
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (!TestAxis(Vector3Cross(axesA[i], axesB[j]))) return false;

        // ---------------- contact data ----------------
        // Ensure normal obeys Collision.h convention: "b to a"
        if (Dot(bestAxis, A.center - B.center) < 0.f)
            bestAxis = -bestAxis;

        out.normal = bestAxis;
        out.penetration = minOverlap;

        // Approximate a single contact point – midpoint of extreme vertices along the normal.
        Float3 pa = SupportVertex(A, -bestAxis);
        Float3 pb = SupportVertex(B, bestAxis);
        out.point = (pa + pb) * 0.5f;

        /*       std::cout << "Collided OBB with OBB: "
                   << "Penetration: " << out.penetration << '\n'
                   << " Normal: " << out.normal.x() << ", " << out.normal.y() << ", " << out.normal.z() << '\n'
                   << " Point: " << out.point.x() << ", " << out.point.y() << ", " << out.point.z() << '\n';*/

        return true; // Boxes intersect.
    }


    //[[nodiscard]] 
    //bool Collide(const AABB& a, const OBB& b, Contact& out) {

    //    //delegate to OBB vs OBB:
    //    OBB mimicOBB;
    //    mimicOBB.center = (a.min + a.max) * 0.5f;
    //    mimicOBB.halfExtents = (a.max - a.min) * 0.5f;
    //    mimicOBB.axis[0] = Float3{ 1, 0, 0 };
    //    mimicOBB.axis[1] = Float3{ 0, 1, 0 };
    //    mimicOBB.axis[2] = Float3{ 0, 0, 1 };
    //    return Collide(mimicOBB, b, out);
    //}


    ////AABB vs OBB
    //[[nodiscard]] 
    //bool Collide(const AABB& a, const OBB& b, Contact& out) {
    //    // convert the AABB
    //    Float3 aCenter = (a.min + a.max) * 0.5f;
    //    Float3 aExtent = (a.max - a.min) * 0.5f;
    //
    //    // constant axis;
    //    const Float3 aAxes[3] = {
    //        Float3{1,0,0}, Float3{0,1,0}, Float3{0,0,1}
    //    };
    //
    //    float   minOverlap = FLT_MAX;
    //    Float3  bestAxis = Float3{ 0,0,0 };
    //     
    //	//capture by reference
    //    auto testAxis = [&](const Float3& axis) {
    //
    //        Interval aProj = AABBProject(a, axis);
    //        Interval bProj = OBBProject(b, axis);
    //
    //        float overlap = 0.0f;
    //        if (!IntervalOverlap(aProj, bProj, overlap)) {
    //            return false;  
    //
    //        }
    //        if (overlap < minOverlap) {
    //            minOverlap = overlap;
    //           
    //            //point B to A
    //			if (Dot(axis, aCenter - b.center) > 0) {
    //				bestAxis = axis;  
    //			}
    //			else {
    //				bestAxis = -axis; 
    //			} 
    //
    //        };
    //        return true;
    //        };
    // 
    //    for (int i = 0; i < 3; i++)
    //        if (!testAxis(aAxes[i]))
    //            return false;
    // 
    //    for (int i = 0; i < 3; i++)
    //        if (!testAxis(b.axis[i]))
    //            return false;
    // 
    //    for (int i = 0; i < 3; i++) {
    //        for (int j = 0; j < 3; j++) {
    //            Float3 axis = Vector3Cross(aAxes[i], b.axis[j]);
    //            float len2 = Dot(axis, axis);
    //            if (len2 < 1e-6f)
    //                continue;   // 共线，跳过
    //			axis = Normalize(axis); // 归一化
    //            if (!testAxis(axis))
    //                return false;
    //        }
    //    }
    // 
    //    out.penetration = minOverlap; 
    //    std::cout << "Collide AABB with OBB: penetration = " << out.penetration << std::endl;
    //
    //	std::cout << "Best axis: " << bestAxis.x() << ", " << bestAxis.y() << ", " << bestAxis.z() << std::endl;
    //
    //	//make sure the normal is pointing from B to A
    //    out.normal = bestAxis;
    //
    //
    //    // 6. 近似计算接触点（在 A 与 B 表面之间做中点）
    //    {
    //		Float3 contactPointA = aCenter - bestAxis * (
    //            aExtent.x() * std::abs(bestAxis.x()) +
    //			aExtent.y() * std::abs(bestAxis.y()) +
    //			aExtent.z() * std::abs(bestAxis.z())
    //            );
    //		Float3 contactPointB = b.center + bestAxis * (
    //            b.halfExtents.x() * std::abs(Dot(b.axis[0], bestAxis)) +
    //			b.halfExtents.y() * std::abs(Dot(b.axis[1], bestAxis)) +
    //			b.halfExtents.z() * std::abs(Dot(b.axis[2], bestAxis))
    //            );
    //		out.point = (contactPointA + contactPointB) * 0.5f; 
    //        std::cout << "Contact point: " << out.point.x() << ", " << out.point.y() << ", " << out.point.z() << std::endl;
    //    }
    //
    //    return true;
    //}
    //
    //// AABB vs OBB
    //[[nodiscard]]
    //bool Collide(const OBB& obb, const AABB& aabb, Contact& out)
    //{
    //	//std::cout << "Collide OBB with AABB" << std::endl;
    //	if (!Collide(aabb, obb, out)) return false;
    //	out.normal *= -1.0f; 
    //	//std::swap(out.a, out.b); 
    //
    //	//std::cout << "Collide OBB with AABB: true normal = " << out.normal.x() << ", " << out.normal.y() << ", " << out.normal.z() << std::endl;
    //     
    //	return true;
    //}




    inline Float3 ProjectToPlane(const PlaneWS& plane, const Float3& point) {
        //project a point onto the plane
        float dist = SignedDist(plane, point);
        return point - plane.normal * dist;
    }

    inline std::array<Float3, 8> GetOBBVertices(const OBB& b) {
        std::array<Float3, 8> verts;
        int idx = 0;
        for (int sx = -1; sx <= 1; sx += 2)
            for (int sy = -1; sy <= 1; sy += 2)
                for (int sz = -1; sz <= 1; sz += 2) {
                    // C + sx*e_x*u_x + sy*e_y*u_y + sz*e_z*u_z
                    verts[idx++] = b.center
                        + b.halfExtents.x() * sx * b.axis[0]
                        + b.halfExtents.y() * sy * b.axis[1]
                        + b.halfExtents.z() * sz * b.axis[2];
                }
        return verts;
    }


    //plane vs OBB
    [[nodiscard]]
    bool Collide(const OBB& b, const PlaneWS& p, Contact& out)
    {
        auto verts = GetOBBVertices(b);

        std::vector<Float3> penetratingVerts;
        std::vector<float> penetrations;

        for (const auto& v : verts) {
            float d = SignedDist(p, v);
            if (d < 0.0f) { // inside or touching
                penetratingVerts.push_back(v);
                penetrations.push_back(-d); // store positive penetration
            }
        }

        if (penetratingVerts.empty())
            return false;

        // ---- Manifold aggregation ----
        Float3 avgPoint = Float3{ 0, 0, 0 };
        float totalPenetration = 0.0f;

        for (size_t i = 0; i < penetratingVerts.size(); ++i) {
            avgPoint += penetratingVerts[i];
            totalPenetration += penetrations[i];
        }

        avgPoint = avgPoint / static_cast<float>(penetratingVerts.size());
        float avgPenetration = totalPenetration / penetratingVerts.size();

        // You could also use max(penetrations) instead of average if you prefer more conservative correction

        // ---- Output Contact ----
        out.point = ProjectToPlane(p, avgPoint); // ensure contact point is on plane
        out.normal = p.normal;
        out.penetration = avgPenetration;
        return true;
    }

    //plane vs OBB
    [[nodiscard]]
    bool Collide(const PlaneWS& plane, const OBB& obb, Contact& out)
    {
        //std::cout << "Collide OBB with Plane" << std::endl;
        if (!Collide(obb, plane, out)) return false;
        out.normal *= -1.0f;
        //std::swap(out.a, out.b);
        return true;
    }