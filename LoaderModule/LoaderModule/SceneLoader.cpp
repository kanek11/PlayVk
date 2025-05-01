#include "SceneLoader.h"

#include <iostream> 
#include <format>
#include <span>

#include <limits>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>




namespace Utils {

	/*
	* assimp is row major, 
	glm/OpenGL expect column major
	*/
	glm::mat4 Mat4_AssimpToGLM(const aiMatrix4x4& matrix){
		glm::mat4 rowMaj = glm::make_mat4(&matrix.a1);  //simply float[16]
		glm::mat4 colMaj = glm::transpose(rowMaj);
		return colMaj;
	}

	constexpr glm::vec3 Vec3_AssimpToGLM(const aiVector3D& vec) {
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	constexpr glm::quat Quat_AssimpToGLM(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}  
}


/*
	todo:  
	handle one material with multiple textures?
	 aiString path;
		if (ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == AI_SUCCESS) {
			cout << "Model Loader: Expect Base Texture Path: " << path.C_Str() << '\n';
		}
*/ 
namespace Loader {

	constexpr uint32_t NONE_INDEX = std::numeric_limits<uint32_t>::max();
	using aiIndex_t = decltype(aiScene::mNumMeshes);



	static void processAnimations(const aiScene* scene, SceneData& sceneData)
	{   
		sceneData.animations = [&] { 
		
			std::vector<Animation> animations{};  

			cout << "Loader: found animation num: " << scene->mNumAnimations << '\n';
			for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
			{  
				Animation animation{};

				aiAnimation* ai_animation = scene->mAnimations[i];
				animation.totalTicks = ai_animation->mDuration;
				animation.ticksPerSecond = ai_animation->mTicksPerSecond;
			 
			 
				for (uint32_t j =  0 ; j < ai_animation->mNumChannels; ++j)
				{
					aiNodeAnim* ai_nodeAnim = ai_animation->mChannels[j];  
				
					animation.trackNames.push_back(ai_nodeAnim->mNodeName.C_Str());

					KeyFrameData keyframe{};
					for (uint32_t k = 0; k < ai_nodeAnim->mNumPositionKeys; ++k)
					{
						aiVectorKey key = ai_nodeAnim->mPositionKeys[k]; 
						keyframe.posTicks.push_back(key.mTime);
						keyframe.keyPositions.push_back(Utils::Vec3_AssimpToGLM(key.mValue));
					}

					for (uint32_t k = 0; k < ai_nodeAnim->mNumRotationKeys; ++k)
					{ 
						aiQuatKey key = ai_nodeAnim->mRotationKeys[k];
						keyframe.rotTicks.push_back(key.mTime);
						keyframe.keyRotations.push_back(Utils::Quat_AssimpToGLM(key.mValue));
					}

					for (uint32_t k = 0; k < ai_nodeAnim->mNumScalingKeys; ++k)
					{
						aiVectorKey key = ai_nodeAnim->mScalingKeys[k]; 
						keyframe.scaleTicks.push_back(key.mTime);
						keyframe.keyScales.push_back(Utils::Vec3_AssimpToGLM(key.mValue));
					}

					animation.rawData.push_back(keyframe);
				}

				animations.push_back(animation);

				cout << "Loader: anim root name:" << ai_animation->mName.C_Str() << '\n';
				cout << "Loader: anim total ticks:" << ai_animation->mDuration << '\n';
				cout << "Loader: anim tick per sec:" << ai_animation->mTicksPerSecond << '\n';
				cout << "Loader: anim tracks:" << ai_animation->mNumChannels << '\n';
			}

			return animations;

		}();

	}


	static void processNodes(const aiScene* scene, aiNode* ai_node, SceneData& sceneData)
	{

		if (!ai_node) { std::cerr << "Loader: Invalid node acesss!\n"; return; }  

		struct Context {
			aiNode* node; 
			uint32_t parentId;
		};
		std::vector<Context> virtualStack{};

		//initial
		virtualStack.push_back(Context{ ai_node, NONE_INDEX });

		while (!virtualStack.empty())
		{
			//routine
			auto& task = virtualStack.back();
			virtualStack.pop_back();

			//unwrap parameters
			aiNode* node = task.node;
			uint32_t parentId = task.parentId;

			//body
			uint32_t thisId = sceneData.nodes.size();

			if (parentId != NONE_INDEX)
				sceneData.nodes[parentId].childrenIndices.push_back(thisId);
			 
			Node thisNode{};
			thisNode.name = node->mName.C_Str();
			thisNode.localTransform = Utils::Mat4_AssimpToGLM(node->mTransformation);

			auto result = std::format("Loader: visit node: name: {0}, assign id: {1}", thisNode.name, thisId);
			std::cout << result << '\n';

			for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
				thisNode.meshIndices.push_back(node->mMeshes[i]);
			}

			sceneData.nodes.push_back(thisNode);  

			for (uint32_t i = 0; i < node->mNumChildren; ++i)
			{
				virtualStack.push_back(Context{ node->mChildren[i], thisId }); 
			} 
		}

		//debug:
		cout << "Loader: Total Node number: " << sceneData.nodes.size() << '\n'; 
		 
	}

 



	

	static void processMaterials(const aiScene* scene, SceneData& sceneData)
	{  
		cout << "Loader: Model Loader: Material number: " << scene->mNumMaterials << '\n';
		std::vector<Material> materials{};
		std::vector<std::string> texturePaths{};

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {

			aiMaterial* ai_material = scene->mMaterials[i];

			Material material{};
			material.name = ai_material->GetName().C_Str();

			cout << "Loader: Material name:" << material.name << '\n';
			cout << "Loader: Material index:" << i << '\n';
			 
			//hack: 遍历所有可能的纹理类型
			for (int textureType = aiTextureType_NONE; textureType <= aiTextureType_UNKNOWN; ++textureType) {
				unsigned int textureCount = ai_material->GetTextureCount(static_cast<aiTextureType>(textureType));

				for (unsigned int j = 0; j < textureCount; ++j) {
					cout << "Loader: Texture count: " << textureCount << '\n';
					aiString texturePath;
					if (ai_material->GetTexture(static_cast<aiTextureType>(textureType), j, &texturePath) == AI_SUCCESS) {
						std::cout << "Loader: Expect Texture path: " << texturePath.C_Str() << '\n';
						 
						//avoid duplicate reference;
						if (auto it = std::find(texturePaths.begin(), texturePaths.end(), texturePath.C_Str());
							it != texturePaths.end()) {
							// Path already exists, use existing index
							size_t existingIndex = std::distance(texturePaths.begin(), it);
							material.textureIndices.push_back(static_cast<uint32_t>(existingIndex));
							cout << "Loader: texture path already exist as:" << existingIndex << '\n';
						}
						else {
							// Path does not exist, add new entry
							texturePaths.push_back(texturePath.C_Str());
							material.textureIndices.push_back(static_cast<uint32_t>(texturePaths.size() - 1));
							cout << "Loader: register path as id:" << texturePaths.size() - 1 << '\n';
						}
					}
				} 
			}

			materials.push_back(material);

		}

		sceneData.materials = materials;
		sceneData.texturePaths = texturePaths;

	}




	static void processBones(aiMesh* mesh, MeshData& meshData)
	{  
		meshData.vertexGroups = [&] { 
			std::vector<VertexGroup> vertexGroups{};
			for (uint32_t i = 0; i < mesh->mNumBones; ++i)
			{
				aiBone* bone = mesh->mBones[i];

				VertexGroup _vertexGroup{};
				_vertexGroup.boneName = bone->mName.C_Str();
				_vertexGroup.offsetMatrix = Utils::Mat4_AssimpToGLM(bone->mOffsetMatrix);

				for (uint32_t j = 0; j < bone->mNumWeights; ++j)
				{
					aiVertexWeight weight = bone->mWeights[j];
					_vertexGroup.weights.push_back({ weight.mVertexId, weight.mWeight });

				}

				vertexGroups.push_back(_vertexGroup);
			}

			return vertexGroups;

		} ();
		 
	} 



    static void processMeshes(const aiScene* scene,  SceneData& sceneData)
	{

		sceneData.meshes = [&] { 
			
			vector<MeshData> meshes{}; 

			std::span<aiMesh*> meshSpan(scene->mMeshes, scene->mNumMeshes); 
			for (aiMesh* ai_mesh : meshSpan) {

				//interface:
				MeshData meshData{};
				meshData.name = ai_mesh->mName.C_Str();
				meshData.numVertices = ai_mesh->mNumVertices;
				meshData.numFaces = ai_mesh->mNumFaces;   
				meshData.materialIndex = ai_mesh->mMaterialIndex; //material index

				for (uint32_t j = 0; j < ai_mesh->mNumVertices; ++j)
				{
					if (ai_mesh->HasPositions())
					meshData.positions.push_back(Utils::Vec3_AssimpToGLM(ai_mesh->mVertices[j]));

					if (ai_mesh->HasTextureCoords(0))
					meshData.UVs.push_back(Utils::Vec3_AssimpToGLM(ai_mesh->mTextureCoords[0][j]));

					if (ai_mesh->HasNormals()) 
					meshData.normals.push_back(Utils::Vec3_AssimpToGLM(ai_mesh->mNormals[j]));

					if (ai_mesh->HasTangentsAndBitangents())
					meshData.tangents.push_back(Utils::Vec3_AssimpToGLM(ai_mesh->mTangents[j]));

				}


				if (ai_mesh->HasFaces())
				{
					auto faceSpan = span<aiFace>(ai_mesh->mFaces, ai_mesh->mNumFaces);
					for (const aiFace& face : faceSpan) {

						auto indexSpan = span<unsigned int>(face.mIndices, face.mNumIndices);
						for (const auto index : indexSpan) {
							meshData.indices.push_back(index);
						}
					}
				}


				//new: getBones
				processBones(ai_mesh, meshData);


				cout << "Loader: mesh loader: name:" << ai_mesh->mName.C_Str() << '\n';
				cout << "Loader: id:" << meshes.size() << '\n';
				cout << "Loader: materialindex: " << ai_mesh->mMaterialIndex << '\n';
				cout << "Loader: numVertices:" << ai_mesh->mNumVertices << '\n';
				cout << "Loader: numFaces:" << ai_mesh->mNumFaces << '\n';
				cout << "Loader: numVertexGroups:" << ai_mesh->mNumBones << '\n';
				meshes.push_back(meshData);
			}


			return meshes;
		 
		
		}();

	}


	std::optional<SceneData> loadSceneData(std::string_view pathView, const SceneLoaderConfig& options)
	{ 
		Assimp::Importer importer{}; 
		importer.SetPropertyFloat("GLOBAL_SCALE_FACTOR", options.scale);
		 
		const aiScene* ai_scene = importer.ReadFile(pathView.data(),
			aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		//a complete scene, with a root node ;
		if (!ai_scene ||  !ai_scene->mRootNode ||  ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE )
		{
			std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString();
			return std::nullopt;
		}


		//interface:
		SceneData sceneData{};

		if (options.expectMeshes)
		{ 
			 processMeshes(ai_scene, sceneData);
		}

		//override the implementation-dependent global scale;
		aiMatrix4x4 newIdentity{};                         // identity by default
		ai_scene->mRootNode->mTransformation = newIdentity; 

        //to extend:
		if (options.expectHierarchy)
		{
			 processNodes(ai_scene, ai_scene->mRootNode, sceneData);
		} 


		if (options.expectMaterials)
		{
			processMaterials(ai_scene, sceneData);

			auto textures = ai_scene->mNumTextures;
			cout << "Loader: found embedded texture num:" << textures << '\n';
		}
		 
		if (options.expectAnimations)
		{
			processAnimations(ai_scene, sceneData);
		}

		return sceneData;
	}



}