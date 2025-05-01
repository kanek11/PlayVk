#pragma once
 
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp> 

#include <limits>  
#include <algorithm>
#include <iostream>

using Scalar_t = float;

using vec3 = glm::vec3;
using vec2 = glm::vec2;

constexpr Scalar_t MAX_SCALAR_V = std::numeric_limits<Scalar_t>::max();
constexpr Scalar_t MIN_SCALAR_V = std::numeric_limits<Scalar_t>::min();


constexpr Scalar_t PI = 3.14159265358979323846f;

inline Scalar_t deg2rad(const Scalar_t& deg) { return deg * PI / 180.0f; }

inline vec3 normalize(const vec3& v) { return glm::normalize(v); }

inline vec3 reflect(const vec3& v, const vec3& n) {
	auto dot = glm::dot(-v, n);
	return v + 2.0f * dot * n;
}


inline float max3(const float& x, const float& y, const float& z) {
	return std::max(std::max(x, y), z);
}

inline float min3(const float& x, const float& y, const float& z) {
	return std::min(std::min(x, y), z);
}

inline void printVec3(const vec3& v) {
	std::cout << v.x << " " << v.y << " " << v.z << '\n';
}