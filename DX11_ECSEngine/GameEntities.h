#pragma once
#include "entt/entt.hpp"
#include <fstream>
#include <DirectXMath.h>
#include <vector>
#include "Components.h"
class GameEntities
{
public:
	GameEntities();
	~GameEntities();

	// -----------------------------------------------------------------------
	// - Iterate 'n' times through all entities having the two mesh components.
	// - Store vb, ib, and indices after reading from the obj file.
	// - Could be improved with ASSIMP.
	// -----------------------------------------------------------------------
	void LoadMesh(const char* objFile, entt::registry& registry);

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
};