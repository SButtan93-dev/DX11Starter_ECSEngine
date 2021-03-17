#pragma once
#include "entt/entt.hpp"
#include "assimp/Importer.hpp"
#include "Assimp/assimp/scene.h"
#include "Assimp/assimp/postprocess.h"
#include <DirectXMath.h>
#include <vector>
#include "Components.h"

#define NUM_BONES_PER_VEREX 4




class GameEntities
{
	struct VertexBoneData
	{
		unsigned int IDs[NUM_BONES_PER_VEREX];
		float Weights[NUM_BONES_PER_VEREX];

		void AddBoneData(UINT BoneID, float Weight, UINT x);
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

public:
	GameEntities();
	~GameEntities();
	std::map<std::string, UINT> mBoneMapping; // maps a bone name to its index
	// -----------------------------------------------------------------------
	// - Iterate 'n' times through all entities having the two mesh components.
	// - Store vb, ib, and indices after reading from the obj file.
	// - Could be improved with ASSIMP.
	// -----------------------------------------------------------------------
	void LoadMesh(const char* objFile, entt::registry& registry);

	void LoadBones(aiMesh* mesh);

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

	UINT mNumBones;
	std::vector<BoneInfo> mBoneInfo;

};