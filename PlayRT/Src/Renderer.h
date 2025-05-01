#pragma once
#include <vector>
#include <memory>

using namespace std;

#include "Math.h"
#include "BVH.h"
#include "Triangle.h"
#include "Ray.h"


constexpr uint32_t WIDTH = 160;
constexpr uint32_t HEIGHT = 90;

  

struct Camera { 
	//uint32_t width, height;
	float fov = 90.0;
};



class Scene {
public:
	void buildBVH();
	optional<Intersection> intersect(const Ray& ray) const;
	
	Camera camera; 
	vector<Triangle> triangles;  
	shared_ptr<BVH<Triangle>> bvh;
};


class Renderer {
public:
	void render(const Scene& scene);  
	vec3 traceRay(const Ray& ray, const Scene& scene, uint32_t depth) const;


	vec3 cameraOrigin{ 0.0f, 0.0f, -1.0 }; 
	vec3 background{ 0.1f };
	vector<vec3> framebuffer; 
	uint32_t maxDepth = 1;
	double russianRoulette = 0.8;
};