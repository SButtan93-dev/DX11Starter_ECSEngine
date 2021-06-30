#include "InitEngine.h"
#include "DXCore.h"
//#include <Windows.h>
	// One single registry for all the entities and components.


InitEngine::InitEngine()
{
}


InitEngine::~InitEngine()
{
	delete Mesh;
}

// ----------------------------------------------------------------------
// - Initialize entities with components and store them in the registry.
// - Call systems to initialize windows, GPU, camera, meshes and texture.
// - Pass registry to game loop.
// ----------------------------------------------------------------------
void InitEngine::InitEntt(RenderWindow mystruct)
{
	
	// Init entities
	entt::entity mainWindow = m_rendererRegistry.create();
	entt::entity camera = m_rendererRegistry.create();
	entt::entity texture = m_rendererRegistry.create();

	// Tag?
	// m_rendererRegistry.assign<entt::tag<"MainWindow"_hs>>(mainWindow);

	// Assign Window components to start the window.
	m_rendererRegistry.emplace<RenderWindow>(mainWindow, mystruct.hInstance, mystruct.titleBarText, mystruct.titleBarStats, nullptr);

	// Assign main GPU vars.
	m_rendererRegistry.emplace<RendererMainVars>(mainWindow, nullptr,
		nullptr,nullptr, nullptr, nullptr, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1);

	// Initialize window screen data
	unsigned int width1 = 1280;
	unsigned int height1 = 720;
	m_rendererRegistry.emplace<RenderWindowDimensions>(mainWindow, width1, height1);

	// FPS data, could be intialized directly instead of using local variable.
	FPSData obj_FPS;
	obj_FPS.fpsFrameCount = 0;
	obj_FPS.fpsTimeElapsed = 0.0f;
	m_rendererRegistry.emplace_or_replace<FPSData>(mainWindow, obj_FPS.fpsFrameCount, obj_FPS.fpsTimeElapsed);

	// Initialize window
	SystemsPlan::Plan->StartWindow(m_rendererRegistry);

	// Initialize DX variables
	SystemsPlan::Plan->InitDirectXVars(m_rendererRegistry);
	
	// Local camera components.
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT4X4 view_m;
	DirectX::XMFLOAT4X4 pro_m;
	DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionIdentity());
	DirectX::XMStoreFloat4x4(&view_m, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&pro_m, DirectX::XMMatrixIdentity());
	m_rendererRegistry.emplace_or_replace<CameraComponents>(camera, view_m, pro_m, DirectX::XMFLOAT3(0.0f,0.0f,0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),rotation,0.0f,0.0f);

	SystemsPlan::Plan->InitShaderVars(m_rendererRegistry);

	SystemsPlan::Plan->LoadCreateShader(m_rendererRegistry);

	SystemsPlan::Plan->LoadCreatePixelShader(m_rendererRegistry);

	SystemsPlan::Plan->LoadCreateShaderSky(m_rendererRegistry); // Sky

	SystemsPlan::Plan->LoadCreatePixelShaderSky(m_rendererRegistry); // Sky

	SystemsPlan::Plan->SetSkyShaderVars(m_rendererRegistry); // Sky

	SystemsPlan::Plan->CreateMatricesGeometry(m_rendererRegistry);

	auto& t_comp = m_rendererRegistry.emplace<TextureData>(texture, nullptr, nullptr, nullptr, nullptr);

	Mesh->InitTexture(m_rendererRegistry);

	// Enter the number of mesh entities
	unsigned int m_count = 1;

	//VertexBoneData bl;
	//bl.IDs.push_back(0);
	// Create empty mesh entities
	for (unsigned int i = 0; i < m_count; i++)
	{
		entt::entity newEntity = m_rendererRegistry.create();
		m_rendererRegistry.emplace<MeshRenderVars>(newEntity, nullptr, nullptr, 0);
		DirectX::XMFLOAT4X4 abc;
		DirectX::XMStoreFloat4x4(&abc, DirectX::XMMatrixIdentity());
		m_rendererRegistry.emplace<MeshEntityData>(newEntity, abc, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		//m_rendererRegistry.emplace<VertexBoneData>(newEntity, bl);
	}

	// Load mesh entities 'm_count' times in the buffers.
	Mesh->LoadMesh("Models/silly_dancing.fbx", m_rendererRegistry);

	entt::entity newEntity = m_rendererRegistry.create();
	m_rendererRegistry.emplace<MeshRenderVarsSky>(newEntity, nullptr, nullptr, 0);
	DirectX::XMFLOAT4X4 abc;
	DirectX::XMStoreFloat4x4(&abc, DirectX::XMMatrixIdentity());
	m_rendererRegistry.emplace<MeshEntityDataSky>(newEntity, abc, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

	// Sky
	Mesh->LoadMeshSky("Models/Cube.obj", m_rendererRegistry);

	// All set, pass the registry 
	// to core game loop system
	SystemsPlan::Plan->RunDXCore(m_rendererRegistry, Mesh);
}

// Try releasing the GPU variables from the entities.
void InitEngine::Clean()
{
	auto ref_CompPS = m_rendererRegistry.view<SimplePixelShaderStruct>();

	for (auto entity : ref_CompPS)
	{
		auto ps = ref_CompPS.get<SimplePixelShaderStruct>(entity);

		ps.shader->Release();
		ps.shader = 0;
		m_rendererRegistry.replace<SimplePixelShaderStruct>(entity, ps.shader);
		m_rendererRegistry.remove_all(entity);
		m_rendererRegistry.destroy(entity);
	}

	auto ref_CompsVS = m_rendererRegistry.view<SimpleVertexShaderStruct>();

	for (auto entity : ref_CompsVS)
	{
		auto vs = ref_CompsVS.get<SimpleVertexShaderStruct>(entity);

		vs.shader->Release();
		vs.shader = 0;

		vs.inputLayout->Release();
		vs.inputLayout = 0;

		m_rendererRegistry.replace<SimpleVertexShaderStruct>(entity, vs.perInstanceCompatible, vs.inputLayout, vs.shader);
		m_rendererRegistry.remove_all(entity);
		m_rendererRegistry.destroy(entity);
	}

	auto ref_BufVS = m_rendererRegistry.view<SimpleShaderVariables>();

	for (auto entity : ref_BufVS)
	{
		auto buf_vs = ref_BufVS.get<SimpleShaderVariables>(entity);

		buf_vs.shaderBlob->Release();
		buf_vs.shaderBlob = 0;

		if (buf_vs.ConstantBuffer != nullptr)
			buf_vs.ConstantBuffer->Release();

		m_rendererRegistry.replace<SimpleShaderVariables>(entity, buf_vs.shaderValid, buf_vs.shaderBlob, buf_vs.ConstantBuffer, buf_vs.constantBufferCount);
		m_rendererRegistry.remove<SimpleShaderVariables>(entity);
		m_rendererRegistry.destroy(entity);
	}

	auto ref_BufPS = m_rendererRegistry.view<SimpleShaderPixelVariables>();

	for (auto entity : ref_BufPS)
	{
		auto buf_ps = ref_BufPS.get<SimpleShaderPixelVariables>(entity);

		buf_ps.shaderBlob->Release();
		buf_ps.shaderBlob = 0;
		if (buf_ps.ConstantBuffer != nullptr)
			buf_ps.ConstantBuffer->Release();

		m_rendererRegistry.replace<SimpleShaderPixelVariables>(entity, buf_ps.shaderValid, buf_ps.shaderBlob, buf_ps.ConstantBuffer, buf_ps.constantBufferCount);
		m_rendererRegistry.remove<SimpleShaderPixelVariables>(entity);
		m_rendererRegistry.destroy(entity);
	}

	auto meshvars = m_rendererRegistry.view<MeshRenderVars>();

	for (auto entity : meshvars)
	{
		auto mesh_vars = meshvars.get<MeshRenderVars>(entity);

		mesh_vars.ib = 0;
		mesh_vars.ib->Release();
		mesh_vars.vb = 0;
		mesh_vars.vb->Release();

		m_rendererRegistry.replace<MeshRenderVars>(entity, mesh_vars.vb, mesh_vars.ib, mesh_vars.numIndices);
		m_rendererRegistry.remove<MeshRenderVars>(entity);
		m_rendererRegistry.destroy(entity);
	}

	// - Clear the registry with or without attached entities.
	// - Ensure the GPU variables are released from the registry, and
	//   those components are released from the entity. If not handled
	//   carefully then there could be access violations.
	// - Additionally always initialize a pointer to 'nullptr' or '0', 
	//   it helps create a safe allocation of memory if the memory 
	//   isnt't released by mistake through garbage collector. 
	m_rendererRegistry.clear();
}
