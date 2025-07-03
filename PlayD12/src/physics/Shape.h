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
	FLOAT3 halfExtents;
};

struct Capsule {
	float radius;
	float height;
};
