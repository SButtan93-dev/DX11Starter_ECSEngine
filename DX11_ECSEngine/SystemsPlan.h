#pragma once
#include "entt/entt.hpp"
#include"DXCore.h"
#include "Components.h"
#include "BasicShader.h"
#include "SkyShader.h"
#include "GameEntities.h"
#include "Camera.h"

class SystemsPlan
{
public:
	SystemsPlan();
	~SystemsPlan();

	static SystemsPlan* Plan; // singleton

	Camera camera;

	void StartWindow(entt::registry &m_mainRendererRegistry);

	void InitDirectXVars(entt::registry& registry);

	entt::registry& InitShaderVars(entt::registry& registry);

	void CreateBasicVertexShader(entt::registry & registry);
	void CreateBasicPixelShader(entt::registry & registry);
	void CreateMatricesGeometry(entt::registry& registry);

	void CreateVertexSkyShader(entt::registry& registry);
	void CreatePixelSkyShader(entt::registry& registry);
	void SetSkyShaderVars(entt::registry& registry);

	void CameraUpdate(CameraComponents* comps, float dt, entt::registry& registry);

	void RunDXCore(entt::registry& registry, GameEntities* obj_MeshBoneData);

};

