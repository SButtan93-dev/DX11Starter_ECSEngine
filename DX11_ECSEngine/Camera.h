#pragma once
#include "entt/entt.hpp"
#include "Components.h"

class Camera
{
public:
	
	// Basic movements
	void Update(CameraComponents* camera_comp,float dt, entt::registry& registry);

	// Moves the camera relative to its orientation
	void MoveRelative(float x, float y, float z, CameraComponents* comps);

	// Moves the camera in world space (not local space)
	void MoveAbsolute(float x, float y, float z, CameraComponents* comps);
	void Rotate(float x, float y, entt::registry& registry);

};

