#include "SkyShader.h"
#include "SystemsPlan.h"

SystemsPlan* SystemsPlan::Plan = 0;

ISimpleShader* s_ShaderObj = new ISimpleShader();

SkyShader* sky_obj = new SkyShader();


SystemsPlan::SystemsPlan()
{
	Plan = this;
}


SystemsPlan::~SystemsPlan()
{
	delete s_ShaderObj;

	delete sky_obj;

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
}


// ---------------------------------------------------------
// Called every frame to update camera transform properties
// ---------------------------------------------------------
void SystemsPlan::CameraUpdate(CameraComponents* comps, float dt, entt::registry& registry)
{
	camera.Update(comps, dt,registry);
}


// ---------------------------
// Enter the main loop system
// ---------------------------
void SystemsPlan::RunDXCore(entt::registry & registry, GameEntities* obj_MeshBoneData)
{
	DXCore::DXCoreInstance->MainRunDX(registry, s_ShaderObj, sky_obj,obj_MeshBoneData);
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
entt::registry& SystemsPlan::CreateBasicVertexShader(entt::registry& registry)
{
	return s_ShaderObj->LoadVertexShaderFile(registry);
}


// -----------------------------------------------------------
// Create pixel shader file using SimpleShader class
// Could be expanded as a library for more pixel shader files
// -----------------------------------------------------------
entt::registry & SystemsPlan::CreateBasicPixelShader(entt::registry & registry)
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


void SystemsPlan::CreateVertexSkyShader(entt::registry& registry)
{
	sky_obj->LoadVertexShaderFileSky(registry);
}


void SystemsPlan::CreatePixelSkyShader(entt::registry& registry)
{
	sky_obj->LoadPixelShaderFileSky(registry);
}

void SystemsPlan::SetSkyShaderVars(entt::registry& registry)
{
	sky_obj->SetSkyVars(registry);
}
