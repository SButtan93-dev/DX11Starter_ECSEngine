#pragma once
#include "entt/entt.hpp"
#include "assimp/Importer.hpp"
#include "Assimp/assimp/scene.h"
#include "Assimp/assimp/postprocess.h"
#include <DirectXMath.h>
#include <vector>
#include "Components.h"

#define NUM_BONES_PER_VEREX 6
//
//


class GameEntities
{
public:
	struct VertexBoneData
	{
		unsigned int IDs[NUM_BONES_PER_VEREX];
		float Weights[NUM_BONES_PER_VEREX];
		void AddBoneData(UINT BoneID, float Weight);
		
	};

	struct BoneInfo
	{
		DirectX::XMMATRIX BoneOffset;
		DirectX::XMMATRIX FinalTransformation;

		//BoneInfo()
		//{
		//	BoneOffset.SetZero();
		//	FinalTransformation.SetZero();
		//}
	};


	GameEntities();
	~GameEntities();
	
	DirectX::XMMATRIX BoneTransform(float TimeInSeconds, std::vector<DirectX::XMFLOAT4X4>& Transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

	// -----------------------------------------------------------------------
	// - Iterate 'n' times through all entities having the two mesh components.
	// - Store vb, ib, and indices after reading from the obj file.
	// - Could be improved with ASSIMP.
	// -----------------------------------------------------------------------
	void LoadMesh(const char* objFile, entt::registry& registry);

	void LoadBones(aiMesh* mesh);

	void AddBoneData2(UINT BoneID, float Weight, int vID);

	void LoadMeshAnim();

	void LoadMeshSky(const char* objFile, entt::registry& registry);

	// ---------------------------------------------------
	// - Loads and saves textures using DX texture library
	// - To load multiple textures, create more entities
	// ---------------------------------------------------
	void InitTexture(entt::registry& registry);
	
	// --------------------------------------------------
    // - Used for test to show that texture components
    //   can be toggled in game loop.
    // - BasicSample, Diffuse & Spec are set to zero.
    // - Call InitTexture() to load the components again.
    // --------------------------------------------------
	void CleanUpTexture(entt::registry& registry);

	// Static because only visible to functions in other files, keep one copy.
	static void Rotate(float x, float y, float z, DirectX::XMFLOAT3* rotation) { rotation->x += x;	rotation->y += y;	rotation->z += z; }
	static void SetPosition(float x, float y, float z, DirectX::XMFLOAT3* position) { position->x = x;	position->y = y;	position->z = z; }
	static void UpdateWorldMatrix(MeshEntityData* obj_meshData);

	DirectX::XMMATRIX NodeTransformation;

	const aiScene* pScene = 0;
	Assimp::Importer importer;
	std::vector<Vertex> verts;
	std::vector<VertexBoneData2> verts2;
	//std::vector<VertexBoneData2> temp_data_bones;
	std::map<std::string, UINT> mBoneMapping; // maps a bone name to its index
	std::vector<VertexBoneData> Bones;
	UINT mNumBones;
	std::vector<BoneInfo> mBoneInfo;
	DirectX::XMMATRIX GlobalInverseTransform;

};