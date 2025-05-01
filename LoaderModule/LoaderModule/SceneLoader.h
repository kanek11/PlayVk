#pragma once
#include <vector>
#include <string>
#include <optional>
#include <string_view>

#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;

/*todo:
* handle variant attributes;
* transforms;
* name lookups
* 
* maybe more context for materials;
* 
* reduce the layer levels, so ugly;
* 

*/


//for now, we can't tell the root node of the skeleton, 
//so the whole scene is candidate for animations;
//otherwise we could narrow down the transform array;
//uint32_t boneRootIndex;

/* decisions that leave to the engine
* 
* assemble the vertex group to per-vertex attributes; 
//
//the engine will need:
//std::map<std::string, uint32_t> nodeIndexMap; //lookup name to index
//std::vector<glm::mat4> localTransforms;   //animation to write 

//std::vector<glm::mat4> globalTransforms;  //to drive the rendering

*/
 




namespace Loader { 
	  
struct VertexGroup
{
	struct Weight
	{
		uint32_t vertexIndex; //local to mesh
		float weight;
	};

	std::string boneName;
	glm::mat4 offsetMatrix;
	std::vector<Weight> weights;
};


struct MeshData
{ 
	std::string name;
	uint32_t materialIndex;
	uint32_t numVertices;
	uint32_t numFaces; 
	//glm::vec3 minAABB, maxAABB;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;

	std::vector<uint32_t> indices;

	std::vector<VertexGroup> vertexGroups;
};


//or track;
struct KeyFrameData { 

	std::vector<double> posTicks;
	std::vector<glm::vec3> keyPositions;

	std::vector<double> rotTicks;
	std::vector<glm::quat> keyRotations;

	std::vector<double> scaleTicks;
	std::vector<glm::vec3> keyScales;
};


//can be derived:
//uint32_t numFrames; 
//engine control : rateScale, interpolation, replay , 

struct Animation {
	//meta
	double totalTicks;  //total ticks;
	double ticksPerSecond;  //ticks per second, sample rate
	
	//decouple the intense data and information aspect;
	std::vector<std::string> trackNames;  
	std::vector<KeyFrameData> rawData;
};


 
struct Node
{
	std::string name;   
	std::vector<uint32_t> meshIndices;

	uint32_t parentIndex;
	std::vector<uint32_t> childrenIndices;
	glm::mat4 localTransform; //rest, bind pose;
};


struct Material
{
	std::string name;
	std::vector<uint32_t> textureIndices; 
};

struct SceneData
{ 
	std::vector<Node> nodes; 

	std::vector<MeshData> meshes;    
	std::vector<Material> materials;
	std::vector<std::string> texturePaths; //no duplicate 

	std::vector<Animation> animations;

};


struct SceneLoaderConfig { 
	float scale{ 1.0f };
	bool expectMeshes = true;
	bool expectMaterials = true;
	bool expectHierarchy = false;
	bool expectAnimations = false; 
};

[[nodiscard]] std::optional<SceneData> loadSceneData(std::string_view path,
	const SceneLoaderConfig& options = SceneLoaderConfig{});




}