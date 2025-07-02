#pragma once
#include "PCH.h"
#include "Math/MMath.h"
#include "Shape.h" 
#include "PhysicsScene.h"

//std::min, max
#include <algorithm>


struct AABB { FLOAT3 min, max; };
struct SphereWS { FLOAT3 center; float radius; };
struct CapsuleWS { FLOAT3 p0, p1; float radius; };
struct PlaneWS {  
    FLOAT3 normal; 
    float d; 
    float width;
    float height; 
    FLOAT3 center;
    FLOAT3 right = { 1, 0, 0 };
    FLOAT3 forward = { 0, 0, 1 }; 
};   // dot(p,n)+d = 0

using WorldShape = std::variant<SphereWS, AABB, CapsuleWS, PlaneWS>;

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
            const FLOAT3 center = (c.body ? c.body->position : FLOAT3{}) + c.localOffset;
            return SphereWS{ center, s.radius };
        } 
        else if constexpr (std::is_same_v<Shape, Box>)
        {
            const FLOAT3 center = (c.body ? c.body->position : FLOAT3{}) + c.localOffset;
            return AABB{ center - s.halfExtents, center + s.halfExtents };
        } 
        else if constexpr (std::is_same_v<Shape, Plane>)
        {
			const FLOAT3 n = FLOAT3{ 0, 1, 0 }; //assuming the plane is horizontal for now  
            const float  d = -Dot(n, (c.body ? c.body->position : FLOAT3{}));
            const FLOAT3 center = (c.body ? c.body->position : FLOAT3{}) + c.localOffset;

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

[[nodiscard]]
bool Collide(const AABB& a, const AABB& b, Contact& out)
{ 
	//std::cout << "Collide AABB with AABB" << std::endl;

    if (a.max.x() < b.min.x() || a.min.x() > b.max.x()) return false;
    if (a.max.y() < b.min.y() || a.min.y() > b.max.y()) return false;
    if (a.max.z() < b.min.z() || a.min.z() > b.max.z()) return false;
     
    float dx = std::min(a.max.x() - b.min.x(), b.max.x() - a.min.x());
    float dy = std::min(a.max.y() - b.min.y(), b.max.y() - a.min.y());
    float dz = std::min(a.max.z() - b.min.z(), b.max.z() - a.min.z());

    FLOAT3 normal;
    float penetration;

    if (dx < dy && dx < dz) {
        normal = (a.min.x() < b.min.x()) ? FLOAT3{ -1, 0, 0 } : FLOAT3{ 1, 0, 0 };
        penetration = dx;
    }
    else if (dy < dz) {
        normal = (a.min.y() < b.min.y()) ? FLOAT3{ 0, -1, 0 } : FLOAT3{ 0, 1, 0 };
        penetration = dy;
    }
    else {
        normal = (a.min.z() < b.min.z()) ? FLOAT3{ 0, 0, -1 } : FLOAT3{ 0, 0, 1 };
        penetration = dz;
    }
     
    FLOAT3 point = (FLOAT3{
        std::max(a.min.x(), b.min.x()),
        std::max(a.min.y(), b.min.y()),
        std::max(a.min.z(), b.min.z())
        } + FLOAT3{
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
[[nodiscard]]
bool Collide(const SphereWS& a, const SphereWS& b, Contact& out)
{
    std::cout << "Collide Sphere with Sphere" << std::endl;

    // 1. 计算球心向量及距离
    FLOAT3 ab = b.center - a.center;
    float  distSq = Dot(ab, ab);
    float  r = a.radius + b.radius;

    // 不相交
    if (distSq >= r * r)
        return false;

    float dist = std::sqrt(distSq);
    float penetration = r - dist; // 始终为正

    // 2. 法线：从 B→A，单位化 (normalize)
    FLOAT3 normal;
    if (dist > 1e-6f)
        normal = (a.center - b.center) / dist;
    else
        normal = FLOAT3{ 1, 0, 0 }; // fallback

    // 3. 接触点：两球表面中点 (midpoint of surface points)
    FLOAT3 surfaceA = a.center - normal * a.radius;
    FLOAT3 surfaceB = b.center + normal * b.radius;
    FLOAT3 point = (surfaceA + surfaceB) * 0.5f;

    // 输出
    out.normal = normal;
    out.point = point;
    out.penetration = penetration;
    return true;
}
 
[[nodiscard]]
bool Collide(const SphereWS& s, const AABB& box, Contact& out)
{
	//std::cout << "Collide Sphere with AABB" << std::endl;

    FLOAT3 closestPoint{
    std::clamp(s.center.x(), box.min.x(), box.max.x()),
    std::clamp(s.center.y(), box.min.y(), box.max.y()),
    std::clamp(s.center.z(), box.min.z(), box.max.z())
    };

    FLOAT3 delta = s.center - closestPoint;
    float distSq = Dot(delta, delta);

    if (distSq > s.radius * s.radius) return false;

    float dist = std::sqrt(distSq);
    FLOAT3 normal = dist > 0.0001f ? delta / dist : FLOAT3{ 0, 1, 0 };  
    FLOAT3 point = closestPoint;

    out.normal = normal;
    out.point = point;
    out.penetration = s.radius - dist;
    return true;
}


[[nodiscard]]
bool Collide(const AABB& box, const SphereWS& s, Contact& out)
{ 
	//std::cout << "Collide AABB with Sphere" << std::endl;

    if (!Collide(s, box, out)) return false;

    out.normal = out.normal *  -1.0f;
    return true;
}


[[nodiscard]]
bool Collide(const AABB& box, const PlaneWS& plane, Contact& out)
{
    const FLOAT3 center = (box.min + box.max) * 0.5f;
    const FLOAT3 extents = (box.max - box.min) * 0.5f;

    float r =
        extents.x() * std::abs(plane.normal.x()) +
        extents.y() * std::abs(plane.normal.y()) +
        extents.z() * std::abs(plane.normal.z());

    float d = Dot(plane.normal, center) + plane.d;

    if (std::abs(d) > r)
        return false;  // no intersection

    // -------- NEW: 投影点是否在 Plane 的矩形内 --------
    FLOAT3 projected = center - plane.normal * d; // 投影到平面
    FLOAT3 localVec = projected - plane.center;

    float u = Dot(localVec, plane.right);
    float v = Dot(localVec, plane.forward);

    if (std::abs(u) > plane.width * 0.5f || std::abs(v) > plane.height * 0.5f)
        return false; // 落点超出 plane 面片边界

  
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

    out.normal = out.normal * -1.0f;
    std::swap(out.a, out.b);
    return true;
}


// Sphere vs Plane
[[nodiscard]]
bool Collide(const SphereWS& s, const PlaneWS& plane, Contact& out)
{
    //std::cout << "Collide Sphere with Plane" << std::endl;

    FLOAT3 local = s.center - plane.center;
    float u = Dot(local, plane.right);
    float v = Dot(local, plane.forward);
    if (std::abs(u) > plane.width * 0.5f || std::abs(v) > plane.height * 0.5f)
        return false;

     
    float signedDist = Dot(plane.normal, s.center) + plane.d;
     
    if (std::abs(signedDist) > s.radius)
        return false;
     
    out.normal = (signedDist < 0.f) ? (plane.normal * -1.f) : plane.normal; 
    out.penetration = s.radius - std::abs(signedDist); 
    out.point = s.center - plane.normal * signedDist; 
    return true;
}

// Plane vs Sphere
[[nodiscard]]
bool Collide(const PlaneWS& plane, const SphereWS& s, Contact& out)
{
    //std::cout << "Collide Plane with Sphere" << std::endl;

    // 调用上面的 Sphere–Plane
    if (!Collide(s, plane, out))
        return false;

    // 反转法线和碰撞双方
    out.normal *= -1.f;
    std::swap(out.a, out.b);

    return true;
}