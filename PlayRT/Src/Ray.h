#pragma once

#include "Math.h"
#include <optional>

struct Ray {
	vec3 origin;
	vec3 direction; 

	Ray() = delete;
	Ray(const vec3& origin, const vec3& direction) : origin(origin), direction(direction) {
		invertDir = 1.0f / direction;
	}
	//Scalar_t t = 0.0;
	//Scalar_t tMin = 0.0;
	float t_max = MAX_SCALAR_V;

	//precompute
	vec3 invertDir;  
};

struct Intersection { 
	//geometry information
	vec3 position;
	vec3 normal;
	float travel_t;
};

 
 