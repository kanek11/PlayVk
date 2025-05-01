#pragma once

#include "Math.h"
#include "BVH.h"
#include <optional>

using namespace std;

 
struct alignas(16) Vertex {
    alignas(16) vec3 position{}; 
};


class Triangle : IRTPrimitive {
public:
	Vertex v0, v1, v2;

	Triangle() = delete;
	Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) : v0(v0), v1(v1), v2(v2) {}

	virtual Bounds3 getBoundingBox() const override {
		Bounds3 bounds;
		bounds.min = glm::min(v0.position, glm::min(v1.position, v2.position));
		bounds.max = glm::max(v0.position, glm::max(v1.position, v2.position));
		return bounds;
	}

	virtual optional<Intersection> intersect(const Ray& ray) const override
	{
		//Moller-Trumbore algorithm
		vec3 e1 = v1.position - v0.position;
		vec3 e2 = v2.position - v0.position;
		vec3 p = cross(ray.direction, e2);
		float det = dot(e1, p);
		if (abs(det) < EPSILON) return nullopt;

		float invDet = 1.0f / det;
		vec3 t = ray.origin - v0.position;
		float u = dot(t, p) * invDet;
		if (u < 0 || u > 1) return nullopt;

		vec3 q = cross(t, e1);
		float v = dot(ray.direction, q) * invDet;
		if (v < 0 || u + v > 1) return nullopt;

		float t0 = dot(e2, q) * invDet;
		if (t0 < 0) return nullopt;

		Intersection isect{};
		isect.position = ray.origin + ray.direction * t0;
		isect.normal = normalize(cross(e1, e2));
		isect.travel_t = t0;
		 
		return isect;

	}


};



class TriangleMesh : IRTPrimitive {
public:
	vector<Triangle> triangles;

	TriangleMesh() = delete;
	TriangleMesh(const vector<Triangle>& triangles) : triangles(triangles) {}

	virtual Bounds3 getBoundingBox() const override {
		Bounds3 bounds;
		for (const auto& triangle : triangles) {
			auto triBounds = triangle.getBoundingBox();
			bounds.min = glm::min(bounds.min, triBounds.min);
			bounds.max = glm::max(bounds.max, triBounds.max);
		}
		return bounds;
	}

	virtual optional<Intersection> intersect(const Ray& ray) const override
	{ 
	}


};