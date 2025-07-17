#pragma once
#include "PCH.h"
#include "Math/MMath.h"


//define the basic shapes;
//design decision:  the position/center should be part of physical body, not collider;


struct Plane {
	float width = 1.0f;
	float height = 1.0f;
};


struct Sphere {
	float radius = 1.0f;
};

struct Box {
	Float3 halfExtents;
};

struct Capsule {
	float radius;
	float height;
};


using ShapeType = std::variant<Plane, Sphere, Box>;


//generic fallback:
template<typename T>
inline Float3x3 MakeInertiaTensor(const T& shape, float mass)
{ 
	std::cerr << "\tMakeInertiaTensor: Unsupported shape type: " << typeid(T).name() << std::endl;
	return Float3x3{}; //return an empty tensor
}

inline Float3x3 MakeInertiaTensor(const Sphere& s, float mass)
{
	//Sphere inertia tensor
	float r = s.radius;
	float i = (2.0f / 5.0f) * mass * r * r; //sphere inertia tensor
	
	Float3x3 result;
	result[0] = { i, 0, 0 };
	result[1] = { 0, i, 0 };
	result[2] = { 0, 0, i };

	return result;
}


inline Float3x3 MakeInertiaTensor(const Box& b, float mass)
{
	//Box inertia tensor
	Float3 halfExtents = b.halfExtents; 
	float ixx = (1.0f / 12.0f) * mass * (4.0f * halfExtents.y() * halfExtents.y() + 4.0f * halfExtents.z() * halfExtents.z());
	float iyy = (1.0f / 12.0f) * mass * (4.0f * halfExtents.x() * halfExtents.x() + 4.0f * halfExtents.z() * halfExtents.z());
	float izz = (1.0f / 12.0f) * mass * (4.0f * halfExtents.x() * halfExtents.x() + 4.0f * halfExtents.y() * halfExtents.y());
	Float3x3 result;
	result[0] = { ixx, 0, 0 };
	result[1] = { 0, iyy, 0 };
	result[2] = { 0, 0, izz };
	return result;
}

//Float3x3 MakeInertiaTensor(const Capsule& c, float mass)
//{
//	//Capsule inertia tensor
//	float r = c.radius;
//	float h = c.height;
//	float i = (1.0f / 12.0f) * mass * (3.0f * r * r + h * h); //capsule inertia tensor
//
//	Float3x3 result;
//	result[0] = { i, 0, 0 };
//	result[1] = { 0, i, 0 };
//	result[2] = { 0, 0, i };
//	return result;
//}

//inline Float3x3 MakeInertiaTensor(const Plane& p, float mass)
//{
//	//Plane inertia tensor
//	float w = p.width;
//	float h = p.height;
//	float ixx = (1.0f / 12.0f) * mass * (h * h + w * w);
//	float iyy = (1.0f / 12.0f) * mass * (h * h + w * w);
//	float izz = 0.0f; //plane has no rotation around z-axis
//	Float3x3 result;
//	result[0] = { ixx, 0, 0 };
//	result[1] = { 0, iyy, 0 };
//	result[2] = { 0, 0, izz };
//	return result;
//}

 
inline Float3x3 MakeInertiaTensor(const ShapeType& shape, float mass) {
	return std::visit(
		[mass](auto const& s) -> Float3x3 { 
			return MakeInertiaTensor(s, mass);
		},
		shape
	);
}