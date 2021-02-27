#pragma once

#include "entt/entt.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include "Components.h"
#include <DirectXMath.h>
#include "GameEntities.h"
#include "SimpleShader.h"
#include "SystemsPlan.h"
#include "Camera.h"
// We can include the correct library files here
// instead of in Visual Studio settings if we want
#pragma comment(lib, "d3d11.lib")

class DXCore
{
public:
	 DXCore();

	~DXCore();

	// Static requirements for OS-level message processing
	static DXCore* DXCoreInstance;

	static LRESULT CALLBACK WindowProc(
		HWND hWnd,		// Window handle
		UINT uMsg,		// Message
		WPARAM wParam,	// Message's first parameter
		LPARAM lParam	// Message's second parameter
		);

	// Internal method for message handling
	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Initialize window using handle and instance
	HRESULT InitWindow(entt::registry &registry);

	// Initialize or attach address to main GPU vars
	HRESULT InitDirectX(entt::registry& registry);

	// --------------------------------------------------------
	// This is the main game loop, handling the following:
	//  - OS-level messages coming in from Windows itself
	//  - Calling update & draw back and forth, forever
	// --------------------------------------------------------
	HRESULT MainRunDX(entt::registry& registry, ISimpleShader* obj_shaderClass);

	void Update(float deltaTime, float totalTime, MeshEntityData* obj_mesh);

	// --------------------------------------------------------------------------
	// Supports:
	// - Rendering calls
	// - Multiple meshes (search each entity in EnTT registry assigned in 
	//	 InitEngine.h, also passed as parameter)
	// - Lighting (Point & Dir)
	// - Texture (Diffuse and Specular maps effect created using BasicSampler)
	// --------------------------------------------------------------------------
	void Draw(RendererMainVars* obj_renderer, ISimpleShader* obj_refShader, 
		MeshEntityData* obj_mesh, CameraComponents* obj_matrices, SimpleShaderVariables* obj_shaderVars, 
		SimpleShaderPixelVariables* obj_pixShaderVars, SimpleVertexShaderStruct* obj_vsStruct, 
		SimplePixelShaderStruct* obj_PSStruct, MeshRenderVars* obj_vbib, entt::registry& registry, TimeData objtime);

	GameEntities Mesh; // systems, no data
	Camera camera;	   // systems, no data

	// Convenience methods for handling mouse input, since we
	// can easily grab mouse input from OS-level messages
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);

private:
	
	// Updates the timer for this frame
	void UpdateTimer();			

	// Puts debug info in the title bar
	void UpdateTitleBarStats(D3D_FEATURE_LEVEL dxFeatureLevelPar, FPSData& obj_FPS, std::string& titleBarText, entt::registry& registry);	
};


