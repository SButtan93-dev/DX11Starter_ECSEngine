#pragma once
#include "entt/entt.hpp"
#include"DXCore.h"
#include "Components.h"
#include "SimpleShader.h"
#include "GameEntities.h"
#include "Camera.h"

class SystemsPlan
{
public:
	SystemsPlan();
	~SystemsPlan();

	static SystemsPlan* Plan;

	void StartWindow(entt::registry &m_mainRendererRegistry);

	void InitDirectXVars(entt::registry& registry);

	

	Camera camera;

	entt::registry& InitShaderVars(entt::registry& registry);

	entt::registry & LoadCreateShader(entt::registry & registry);

	entt::registry & LoadCreatePixelShader(entt::registry & registry);

	entt::registry & CreateMatricesGeometry(entt::registry& registry);

	//entt::registry& CreateVbIb(entt::registry& registry);

	//entt::registry& CreateModelGeometry(entt::registry& registry);

//	void CleanUp(entt::registry& registry);

//	entt::registry& GetUpdatedRegistryDXCore(entt::registry& registry);

	void CameraUpdate(CameraComponents* comps, float dt, entt::registry& registry);

	//entt::registry& InitTexture(entt::registry& registry);

	void RunDXCore(entt::registry& registry);

	//GameEntities* e_gameObj = new GameEntities();
};

