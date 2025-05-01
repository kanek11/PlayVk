#include "Renderer.h"

 
void Scene::buildBVH()
{
	this->bvh = make_shared<BVH<Triangle>>(triangles);
}

optional<Intersection> Scene::intersect(const Ray& ray) const
{
	if (!bvh) throw runtime_error("BVH is not built"); 

	return bvh->intersect(ray);
}



void Renderer::render(const Scene& scene)
{
	framebuffer.resize(WIDTH * HEIGHT);

	float HeightScale = tan(deg2rad(scene.camera.fov) / 2.0f);
	float WidthScale = HeightScale * (float)WIDTH / HEIGHT;

	for (uint32_t j = 0; j < HEIGHT; ++j) {
		for (uint32_t i = 0; i < WIDTH; ++i) {

			vec2 screenCoord = vec2(i, j) / vec2(WIDTH, HEIGHT) * 2.0f - 1.0f; 
			screenCoord.x = WidthScale * screenCoord.x;
			screenCoord.y = HeightScale * screenCoord.y;
 
			vec3 rayDir =  normalize(vec3(screenCoord.x, screenCoord.y, 1.0f));

			Ray ray{ cameraOrigin , rayDir};

			framebuffer[j * WIDTH + i] = traceRay(ray, scene, 0);
		}
	} 
}


vec3 Renderer::traceRay(const Ray& ray, const Scene& scene, uint32_t depth) const
{
	if (depth > maxDepth) return vec3(0.0); 
	
	vec3 color(background);
	if (auto isetOpt = scene.intersect(ray); isetOpt.has_value()) {
          
		auto iset = isetOpt.value();
        vec3 normal = normalize(iset.normal);
		vec3 newDir = reflect(ray.direction, normal);
		Ray newRay{ iset.position + normal * 0.001f, newDir };

		//todo: shading
		color = vec3(1.0f);
		 
        color += traceRay(newRay, scene, depth + 1);

		//cout << "actual hit!" << '\n';
        return color;  
	} 

	return color;
}


