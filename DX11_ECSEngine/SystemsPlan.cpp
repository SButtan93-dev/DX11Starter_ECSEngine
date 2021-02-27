#include "SystemsPlan.h"

SystemsPlan* SystemsPlan::Plan = 0;

ISimpleShader* s_ShaderObj = new ISimpleShader();

SystemsPlan::SystemsPlan()
{
	Plan = this;
}


SystemsPlan::~SystemsPlan()
{
	//delete e_gameObj;

	delete s_ShaderObj;

	//DXCore::DXCoreInstance->~DXCore();
	//s_ShaderObj.s_shaderVecs->constantBuffers->ConstantBuffer->Release();
	//s_ShaderObj.s_shaderVecsPixel.constantBuffers->ConstantBuffer->Release();
}


// ------------------------------------------------------------------------
// First main system to start window based on initialized entity - components.  
// Contains RenderWindow, RenderMainVars and RendererWindowDimenseions structs.
// -------------------------------------------------------------------------
void SystemsPlan::StartWindow(entt::registry &m_mainRendererRegistry)
{
	DXCore::DXCoreInstance->InitWindow(m_mainRendererRegistry);
}


// -----------------------------------------------------------------------------------------------
// Initialize DirectX with existing components initialized as part of the DXCore skeleton earlier
// -----------------------------------------------------------------------------------------------
void SystemsPlan::InitDirectXVars(entt::registry& registry)
{
	DXCore::DXCoreInstance->InitDirectX(registry);
	//GameEntities::Entities->InitVBIB();
}


// ------------------------------------------------------------------------
// More of a high level function to return the updated components from DXCore.
// Includes device, context, swapchain, depthbuffer, stencilview, width, height,
// hinstance, titbar stats and titlebar text.
// -------------------------------------------------------------------------
//entt::registry & SystemsPlan::GetUpdatedRegistryDXCore(entt::registry & registry)
//{
//	return DXCore::DXCoreInstance->UpdateRegistryDXCore(registry);
//}


// ---------------------------------------------------------
// Called every frame to update camera transform properties
// ---------------------------------------------------------
void SystemsPlan::CameraUpdate(CameraComponents* comps, float dt, entt::registry& registry)
{
	camera.Update(comps, dt,registry);
}


// ---------------------------------------------------
// Call this system to add more textures for ps and vs
// ---------------------------------------------------
//entt::registry& SystemsPlan::InitTexture(entt::registry& registry)
//{
//	
//	return GameEntities::Entities->InitTexture(registry);
//}


// ---------------------------
// Enter the main loop system
// ---------------------------
void SystemsPlan::RunDXCore(entt::registry & registry)
{
	DXCore::DXCoreInstance->MainRunDX(registry, s_ShaderObj);
}


//------------------------------------------------------------------------------
// Initialize most of the shader varibales.
//------------------------------------------------------------------------------
entt::registry& SystemsPlan::InitShaderVars(entt::registry& registry)
{
	return s_ShaderObj->InitShaderBegin(registry);
}


// -----------------------------------------------------------
// Create vertex shader file using SimpleShader class
// Could be expanded as a library for more vertex shader files
// -----------------------------------------------------------
entt::registry& SystemsPlan::LoadCreateShader(entt::registry& registry)
{
	return s_ShaderObj->LoadShaderFile(registry);
}


// -----------------------------------------------------------
// Create pixel shader file using SimpleShader class
// Could be expanded as a library for more pixel shader files
// -----------------------------------------------------------
entt::registry & SystemsPlan::LoadCreatePixelShader(entt::registry & registry)
{
	return s_ShaderObj->LoadPixelShaderFile(registry);
}


// ---------------------------------------------
// Create projection matrix and set perspective
// ---------------------------------------------
entt::registry & SystemsPlan::CreateMatricesGeometry(entt::registry & registry)
{
	return s_ShaderObj->CreateMatrices(registry);
}


// --------------
// Create meshes
// --------------
//entt::registry & SystemsPlan::CreateModelGeometry(entt::registry & registry)
//{
//	
//	//GameEntities::Entities->LoadMesh("Models/Sphere.obj", registry);
//
//	return registry;
//}

//void SystemsPlan::CleanUp(entt::registry& registry)
//{
//	//GameEntities::Entities->CleanUp(registry);
//}