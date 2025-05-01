
#include "RT.h"

#include <fstream>
#include <sstream>
#include <chrono>

#include "Timer.h"

#include <SceneLoader/SceneLoader.h>


string getTimeString()
{
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);
	std::tm localTime;
	localtime_s(&localTime, &time); //windows
	std::ostringstream oss;
	oss << std::put_time(&localTime, "%Y%m%d_%H%M%S");

	return oss.str();
}

void saveToImage(const std::string& filename, const std::vector<vec3>& framebuffer) {
	if (std::ofstream ofs(filename, std::ios::out | std::ios::binary); ofs) {
		std::cout << "render: " << filename << " is created" << std::endl;
		ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

		for (const auto& pixel : framebuffer) {
			unsigned char color[3] = {
				static_cast<unsigned char>(255 * std::pow(std::clamp(pixel.x, 0.0f, 1.0f), 0.6f)),
				static_cast<unsigned char>(255 * std::pow(std::clamp(pixel.y, 0.0f, 1.0f), 0.6f)),
				static_cast<unsigned char>(255 * std::pow(std::clamp(pixel.z, 0.0f, 1.0f), 0.6f))
			};
			ofs.write(reinterpret_cast<char*>(color), 3);
		}
	}
	else {
		std::cerr << "render: Error: could not open file for ppm output\n";
	}
}

int main() {

	//test:
	vector<Triangle> triangles{};

	auto triangle = Triangle(
		Vertex{ vec3{ 0.0f, -0.5f, 0.0f } },
		Vertex{ vec3{ 0.5f,  0.5f, 0.0f } },
		Vertex{ vec3{-0.5f,  0.5f, 0.0f } });
	triangles.push_back(triangle);

	{ 
		auto bounds = triangle.getBoundingBox();
		cout << "build bounding box" << '\n'; 
		printVec3(bounds.max);
		printVec3(bounds.min);

		auto ray = Ray{ vec3{0.0f,0.0f,1.0f}, normalize(vec3{0.1f, 0.1f, -1.0f}) }; 
		
		 
		if (AABB_intersect(bounds, ray)){
			std::cout << "hit bounding box!" << '\n';
		} 
		else {
			std::cout << "miss bounding box!" << '\n';
		} 
		
		
		auto iset= triangle.intersect(ray);
		if (iset.has_value()) {
			std::cout << "hit triangle!" << '\n';
			auto is = iset.value();
			printVec3(is.position);
		}
		else {
			std::cout << "miss triangle!" << '\n';
		}
		

		auto bvh = BVH(triangles); 

		cout << "BVH build complete" << '\n';
		cout << "Nodes: " << bvh.nodes.size() << '\n';

		iset = bvh.intersect(ray);
		if (iset.has_value()) {
			std::cout << "hit bvh!" << '\n';
			auto is = iset.value();
			printVec3(is.position);
		}
		else {
			std::cout << "miss bvh!" << '\n';
		} 
		 
	}

	if(true)
	{
		Scene scene;
		scene.triangles = triangles;

		{
			Timer timer;
			scene.buildBVH();

			cout << "build bvh: ";
		} 

		Renderer renderer;
		{
			Timer timer;
			renderer.render(scene);  
			cout << "render: ";
		} 
		 
		saveToImage("output_" + getTimeString() + ".ppm", renderer.framebuffer);
	}


	

}