#include "Camera.h"
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

// Basic movements
void Camera::Update(CameraComponents* camera_comp,float dt, entt::registry& registry)
{
	// Current speed
	float speed = dt * 10;

	// Speed up or down as necessary
	if (GetAsyncKeyState(VK_SHIFT)) { speed *= 5; }
	if (GetAsyncKeyState(VK_CONTROL)) { speed *= 0.1f; }

	// Movement
	if (GetAsyncKeyState('W') & 0x8000) { 
		MoveRelative(0, 0, speed, camera_comp); }
	if (GetAsyncKeyState('S') & 0x8000) { MoveRelative(0, 0, -speed, camera_comp); }
	if (GetAsyncKeyState('A') & 0x8000) { MoveRelative(-speed, 0, 0, camera_comp); }
	if (GetAsyncKeyState('D') & 0x8000) { MoveRelative(speed, 0, 0, camera_comp); }
	if (GetAsyncKeyState('X') & 0x8000) { MoveAbsolute(0, -speed, 0, camera_comp); }
	if (GetAsyncKeyState(' ') & 0x8000) { MoveAbsolute(0, speed, 0, camera_comp); }

	// Check for reset
	if (GetAsyncKeyState('R') & 0x8000)
	{
		camera_comp->position = camera_comp->startPosition;
		camera_comp->xRotation = 0;
		camera_comp->xRotation = 0;
		XMStoreFloat4(&camera_comp->rotation, DirectX::XMQuaternionIdentity());
	}

	// Rotate the standard "forward" matrix by our rotation
	// This gives us our "look direction"
	DirectX::XMVECTOR dir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), DirectX::XMLoadFloat4(&camera_comp->rotation));

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
		DirectX::XMLoadFloat3(&camera_comp->position),
		dir,
		DirectX::XMVectorSet(0, 1, 0, 0));

	DirectX::XMStoreFloat4x4(&camera_comp->viewMatrix, DirectX::XMMatrixTranspose(view));

	auto meshEntityComp = registry.view<CameraComponents>();

	for (auto entity : meshEntityComp)
	{
		registry.replace<CameraComponents>(entity, 
											camera_comp->viewMatrix, camera_comp->projMatrix, 
											camera_comp->startPosition, camera_comp->position, 
											camera_comp->rotation, camera_comp->xRotation, 
											camera_comp->yRotation);
	}
}

// Moves the camera relative to its orientation
void Camera::MoveRelative(float x, float y, float z, CameraComponents* comps)
{
	// Rotate the desired movement vector
	DirectX::XMVECTOR dir = DirectX::XMVector3Rotate(DirectX::XMVectorSet(x, y, z, 0), DirectX::XMLoadFloat4(&comps->rotation));

	// Move in that direction
	DirectX::XMStoreFloat3(&comps->position, DirectX::XMLoadFloat3(&comps->position) + dir);  
}

// Moves the camera in world space (not local space)
void Camera::MoveAbsolute(float x, float y, float z, CameraComponents* comps)
{
	// Simple add, no need to load/store
	comps->position.x += x;
	comps->position.y += y;
	comps->position.z += z;
}

// Rotate on the X and/or Y axis
void Camera::Rotate(float x, float y, entt::registry& registry)
{
	auto meshEntityComp = registry.view<CameraComponents>();

	for (auto entity : meshEntityComp)
	{
		auto cameraComponents = registry.get<CameraComponents>(entity);
		// Adjust the current rotation
		cameraComponents.xRotation += x;
		cameraComponents.yRotation += y;

		// Clamp the x between PI/2 and -PI/2
		cameraComponents.xRotation = max(min(cameraComponents.xRotation, XM_PIDIV2), -XM_PIDIV2);

		// Recreate the quaternion
		XMStoreFloat4(&cameraComponents.rotation, XMQuaternionRotationRollPitchYaw(cameraComponents.xRotation, cameraComponents.yRotation, 0));

		registry.replace<CameraComponents>(entity, 
											cameraComponents.viewMatrix, cameraComponents.projMatrix, 
											cameraComponents.startPosition, cameraComponents.position, 
											cameraComponents.rotation, cameraComponents.xRotation, 
											cameraComponents.yRotation);
	}
}