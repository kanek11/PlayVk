#include <ranges> 
#include <string_view>
#include <iostream>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "SceneLoader.h"

using namespace std;

using namespace Loader;

std::string norm_path(std::string_view path) {
	std::string normalized_path(path);  // 从 string_view 构造一个 std::string 副本
	std::ranges::replace(normalized_path, '\\', '/');
	return normalized_path;
}

int main(){

	aiFace face;
	aiMesh mesh;   
	 

	try {
		auto sceneOpt =loadSceneData(norm_path(R"(D:\CG_resources\dae\vampire\dancing_vampire.dae)"), SceneLoaderConfig{});
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl; 
	}

}