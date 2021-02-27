#pragma once
#include <entt/entt.hpp>
#include <Windows.h>
#include "Components.h"
#include "SystemsPlan.h"


class InitEngine
{
public:

	InitEngine();
	~InitEngine();

	entt::registry m_rendererRegistry;
	GameEntities* Mesh = new GameEntities();

	// ----------------------------------------------------------------------
	// - Initialize entities with components and store them in the registry.
	// - Call systems to initialize windows, GPU, camera, meshes and texture.
	// - Pass registry to game loop.
	// ----------------------------------------------------------------------
	void InitEntt(RenderWindow mystruct);
	void Clean();
};


