#include "DXCore.h"
#include <WindowsX.h>
#include <sstream>

// Define the static instance variable so our OS-level 
// message handling function below can talk to our object
DXCore* DXCore::DXCoreInstance = 0;

TimeData obj_time;

POINT prevMousePos;

// store address of the registry to pass in windows calls
entt::registry* registry2 = 0; 

ID3D11Debug* m_d3dDebug = 0; // only used in debug mode

// --------------------------------------------------------
// The global callback function for handling windows OS-level messages.
//
// This needs to be a global function (not part of a class), but we want
// to forward the parameters to our class to properly handle them.
// --------------------------------------------------------
LRESULT DXCore::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DXCoreInstance->ProcessMessage(hWnd, uMsg, wParam, lParam);
}

// Save a static reference to this object.
//  - Since the OS-level message function must be a non-member (global) function, 
//    it won't be able to directly interact with our DXCore object otherwise.
//  - (Yes, a singleton might be a safer choice here).
DXCore::DXCore()
{
	DXCoreInstance = this;
	delete registry2;
}

// --------------------------------------------------------
// Destructor - Clean up (release) all DirectX references
// --------------------------------------------------------
DXCore::~DXCore()
{
}


// -----------------------------------------------------------------
// - Create the window for our application
// - Called once on start of engine for this system with registry
// ----------------------------------------------------------------
HRESULT DXCore::InitWindow(entt::registry &registry)
{
	// faster cache?
	auto mainWinComp = registry.view<RenderWindow, RendererMainVars, RenderWindowDimensions>();

	for (auto entity : mainWinComp)
	{
		auto[win_comp, gpu_comp, win_dim] = mainWinComp.get<RenderWindow, RendererMainVars, RenderWindowDimensions>(entity);
	
		// Start window creation by filling out the
		// appropriate window class struct
		WNDCLASS wndClass = {}; // Zero out the memory
		wndClass.style = CS_HREDRAW | CS_VREDRAW;	// Redraw on horizontal or vertical movement/adjustment
		wndClass.lpfnWndProc = DXCore::WindowProc; // attach windows call events
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = win_comp.hInstance;	// Our app's handle
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// Default icon
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		// Default arrow cursor
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = "Direct3DWindowClass";

		// Attempt to register the window class we've defined
		if (!RegisterClass(&wndClass))
		{
			// Get the most recent error
			DWORD error = GetLastError();

			// If the class exists, that's actually fine.  Otherwise,
			// we can't proceed with the next step.
			if (error != ERROR_CLASS_ALREADY_EXISTS)
				return HRESULT_FROM_WIN32(error);
		}

		// Adjust the width and height so the "client size" matches
		// the width and height given (the inner-area of the window)
		RECT clientRect;
		SetRect(&clientRect, 0, 0, win_dim.width, win_dim.height);
		AdjustWindowRect(
			&clientRect,
			WS_OVERLAPPEDWINDOW,	// Has a title bar, border, min and max buttons, etc.
			false);					// No menu bar

		// Center the window to the screen
		RECT desktopRect;
		GetClientRect(GetDesktopWindow(), &desktopRect);
		int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
		int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

		// Actually ask Windows to create the window itself
		// using our settings so far.  This will return the
		// handle of the window, which we'll keep around for later
		win_comp.hWnd = CreateWindow(
			wndClass.lpszClassName,
			win_comp.titleBarText.c_str(),
			WS_OVERLAPPEDWINDOW,
			centeredX,
			centeredY,
			clientRect.right - clientRect.left,	// Calculated width
			clientRect.bottom - clientRect.top,	// Calculated height
			0,			// No parent window
			0,			// No menu
			win_comp.hInstance,	// The app's handle
			0);			// No other windows in our application

		// Ensure the window was created properly
		if (win_comp.hWnd == NULL)
		{
			DWORD error = GetLastError();
			return HRESULT_FROM_WIN32(error);
		}

		// The window exists but is not visible yet
		// We need to tell Windows to show it, and how to show it
		ShowWindow(win_comp.hWnd, SW_SHOW);

		registry.replace<RenderWindow>(entity, win_comp.hInstance, win_comp.titleBarText, win_comp.titleBarStats, win_comp.hWnd);
	}
	// Return an "everything is ok" HRESULT value
	return S_OK;
}

// -----------------------------------------------------------------
// - Initializes DirectX, which requires a window.  This method
//   also creates several DirectX objects we'll need to start
//   drawing things to the screen.
// -----------------------------------------------------------------
HRESULT DXCore::InitDirectX(entt::registry& registry)
{
	// This will hold options for DirectX initialization
	unsigned int deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// If we're in debug mode in visual studio, we also
	// want to make a "Debug DirectX Device" to see some
	// errors and warnings in Visual Studio's output window
	// when things go wrong!
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// faster due to checking multiple components?
	auto mainWinComp = registry.view<RenderWindow, RendererMainVars, RenderWindowDimensions>(); 

	for (auto entity : mainWinComp)
	{
		auto [mycomp, mycomp2, mycomp3] = mainWinComp.get<RenderWindow, RendererMainVars, RenderWindowDimensions>(entity);

		// Create a description of how our swap
		// chain should work
		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = 1;
		swapDesc.BufferDesc.Width = mycomp3.width;
		swapDesc.BufferDesc.Height = mycomp3.height;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.Flags = 0;
		swapDesc.OutputWindow = mycomp.hWnd;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapDesc.Windowed = true;

		// Result variable for below function calls
		HRESULT hr = S_OK;

		// Attempt to initialize DirectX
		hr = D3D11CreateDeviceAndSwapChain(
			0,							// Video adapter (physical GPU) to use, or null for default
			D3D_DRIVER_TYPE_HARDWARE,	// We want to use the hardware (GPU)
			0,							// Used when doing software rendering
			deviceFlags,				// Any special options
			0,							// Optional array of possible verisons we want as fallbacks
			0,							// The number of fallbacks in the above param
			D3D11_SDK_VERSION,			// Current version of the SDK
			&swapDesc,					// Address of swap chain options
			&(mycomp2.swapChain),					// Pointer to our Swap Chain pointer
			&(mycomp2.device),					// Pointer to our Device pointer
			&(mycomp2.dxFeatureLevel),			// This will hold the actual feature level the app will use
			&(mycomp2.context));					// Pointer to our Device Context pointer
		if (FAILED(hr)) return hr;

		mycomp2.device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug));
		// The above function created the back buffer render target
		// for us, but we need a reference to it
		ID3D11Texture2D* backBufferTexture;
		mycomp2.swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)&backBufferTexture);

		// Now that we have the texture, create a render target view
		// for the back buffer so we can render into it.  Then release
		// our local reference to the texture, since we have the view.
		mycomp2.device->CreateRenderTargetView(
			backBufferTexture,
			0,
			&mycomp2.backBufferRTV);

		backBufferTexture->Release();

		// Set up the description of the texture to use for the depth buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.Width = mycomp3.width;
		depthStencilDesc.Height = mycomp3.height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;

		// Create the depth buffer and its view, then 
		// release our reference to the texture
		ID3D11Texture2D* depthBufferTexture;
		mycomp2.device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
		mycomp2.device->CreateDepthStencilView(depthBufferTexture, 0, &mycomp2.depthStencilView);
		depthBufferTexture->Release();

		// Bind the views to the pipeline, so rendering properly 
		// uses their underlying textures
		mycomp2.context->OMSetRenderTargets(1, &mycomp2.backBufferRTV, mycomp2.depthStencilView);

		// Lastly, set up a viewport so we render into
		// to correct portion of the window
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)mycomp3.width;
		viewport.Height = (float)mycomp3.height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		mycomp2.context->RSSetViewports(1, &viewport);

		registry.replace<RendererMainVars>(entity, mycomp2.swapChain, mycomp2.device, mycomp2.context, mycomp2.backBufferRTV, 
			mycomp2.depthStencilView, mycomp2.dxFeatureLevel);

	}
	// Return the "everything is ok" HRESULT value
	return S_OK;
}

// --------------------------------------------------------
// This is the main game loop, handling the following:
//  - OS-level messages coming in from Windows itself
//  - Calling update & draw back and forth, forever
// --------------------------------------------------------
HRESULT DXCore::MainRunDX(entt::registry& registry, ISimpleShader* obj_shaderClass, SkyShader* obj_SkyShader)
{	
 	// Grab the start time now that
	// the game loop is running
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	obj_time.startTime = now;
	obj_time.currentTime = now;
	obj_time.previousTime = now;

	// Query performance counter for accurate timing information
	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	obj_time.perfCounterSeconds = 1.0 / (double)perfFreq;

	auto mycomp = registry.view<RenderWindow, FPSData, RendererMainVars>();

	auto mycompMesh = registry.view<MeshEntityData, MeshRenderVars>(); // mesh

	auto mycompShader = registry.view<CameraComponents>(); // camera

	auto mycompVertexPixel = registry.view<SimpleShaderVariables, SimpleShaderPixelVariables,
										SimpleVertexShaderStruct, SimplePixelShaderStruct>();
	// For mesh Data
	entt::entity entity1;

	//For renderer Data
	entt::entity entity2;

	// For shader Data
	entt::entity entity3;

	// For vs, ps Data
	entt::entity entity4;

	// For camera data
	entt::entity camera_entity;

	// Search the first mesh, used for test
	for (auto entity : mycompMesh)
	{
		entity1 = entity;
	}
	auto [mycomp4,mycomp3] = registry.get<MeshEntityData, MeshRenderVars>(entity1);

	// Search GPU vars
	for (auto entity : mycomp)
	{
		entity2 = entity;
	}
	auto mycompRender = registry.get<RendererMainVars>(entity2);
	
	// Search camera properties
	for (auto entity : mycompShader)
	{
		entity3 = entity;	
	}
	auto mycompShaderRender  = registry.get<CameraComponents>(entity3); 

	// Search vs and ps components
	for (auto entity : mycompVertexPixel)
	{
		entity4 = entity;
	}
	auto [mycompshaderstruct, mycompPixel, mycompvsStruct, mycompPSStruct] = registry.get<SimpleShaderVariables, SimpleShaderPixelVariables, SimpleVertexShaderStruct, SimplePixelShaderStruct>(entity4);

	// Our overall game and message loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Determine if there is a message waiting
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate and dispatch the message
			// to our custom WindowProc function
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Update timer and title bar (if necessary)
			UpdateTimer();

			// Window titlebar info
			for (auto entity : mycomp)
			{
				auto mycomp_render_main = registry.view<RenderWindow, FPSData>();
				for (auto entity : mycomp_render_main)
				{
					entity2 = entity;				
				}
				auto [mycomp1, mycomp2] = mycomp.get<RenderWindow, FPSData>(entity);
				if (&mycomp1.titleBarStats)
					UpdateTitleBarStats(D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1, mycomp2, mycomp1.titleBarText, registry);
			}

			// Texture toggle
			if (GetAsyncKeyState('O') & 0x8000)
			{
				Mesh.CleanUpTexture(registry);
			}
			else if (GetAsyncKeyState('I') & 0x8000)
			{
				Mesh.InitTexture(registry);
			}

			if (GetAsyncKeyState('M') & 0x8000)
			{
				//SystemsPlan::Plan->CreateModelGeometry(registry);
			}

			// Camera
			auto meshEntityComp = registry.view<CameraComponents>();
			for (auto entity : meshEntityComp)
			{
				camera_entity = entity;		
			}
			auto cameraComponents = registry.get<CameraComponents>(camera_entity); 
			SystemsPlan::Plan->CameraUpdate(&cameraComponents, obj_time.deltaTime, registry); 
			registry2 = &registry;

			// The game loop
			// Update(obj_time.deltaTime, obj_time.totalTime, &mycomp4);

			Draw(&mycompRender, obj_shaderClass, &mycomp4, &cameraComponents, &mycompshaderstruct, &mycompPixel, &mycompvsStruct, &mycompPSStruct, &mycomp3, registry, obj_time, obj_SkyShader);
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------
	// ------------------------------------Break loop and start manual clean after window exits-------------------------------
	// -----------------------------------------------------------------------------------------------------------------------

	// -----------------
	// Release textures
	// -----------------
	auto t_CompId = registry.view<TextureData>();

    for (auto entity : t_CompId)
    {
		auto t_comp = t_CompId.get<TextureData>(entity);
		
		if(t_comp.crateSRV != nullptr)
		t_comp.crateSRV->Release();
		t_comp.crateSRV = 0;
		
		if (t_comp.rustSRV != nullptr)
		t_comp.rustSRV->Release();
		t_comp.rustSRV = 0;
		
		if (t_comp.sampler != nullptr)
		t_comp.sampler->Release();
		t_comp.sampler = 0;
		
		if (t_comp.specSRV != nullptr)
		t_comp.specSRV->Release();
		t_comp.specSRV = 0;
		
		registry.replace<TextureData>(entity, t_comp.crateSRV, t_comp.rustSRV, t_comp.specSRV, t_comp.sampler);
		registry.remove<TextureData>(entity);
		registry.destroy(entity);
    }
    
	// ---------------------
	// Release Mesh buffers
	// ---------------------
	auto m_CompId = registry.view<MeshRenderVars>();

	for (auto entity : m_CompId)
	{
		auto& m_comp = m_CompId.get<MeshRenderVars>(entity);

		ID3D11Buffer* abc = m_comp.vb;
		abc->Release();
		abc = 0;

		ID3D11Buffer* abc2 = m_comp.ib;
		abc2->Release();
		abc2 = 0;

		registry.replace<MeshRenderVars>(entity, abc, abc2, m_comp.numIndices);
		registry.remove<MeshRenderVars>(entity);
		registry.destroy(entity);
	}

	// ---------------------
	// Release main GPU vars
	// ---------------------
	mycompRender.context->Release();
	mycompRender.context = 0;
	mycompRender.depthStencilView->Release();
	mycompRender.depthStencilView = 0;
	mycompRender.device->Release();
	mycompRender.device = 0;
	mycompRender.backBufferRTV->Release();
	mycompRender.backBufferRTV = 0;
	mycompRender.swapChain->Release();
	mycompRender.swapChain = 0;
	registry.replace<RendererMainVars>(entity2, mycompRender.swapChain, mycompRender.device, mycompRender.context, mycompRender.backBufferRTV, mycompRender.depthStencilView, mycompRender.dxFeatureLevel);
	registry.remove_all(entity2);

	// Uncomment below line for debug purpose
	/*
	ID3D11Debug::ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	m_d3dDebug->Release();
	*/

	return E_NOTIMPL;
}



// --------------------------------------------------------------------------
// Supports:
// - Rendering calls
// - Multiple meshes (search each entity in EnTT registry assigned in 
//	 InitEngine.h, also passed as parameter)
// - Lighting (Point & Dir)
// - Texture (Diffuse and Specular maps effect created using BasicSampler)
// --------------------------------------------------------------------------
void DXCore::Draw(RendererMainVars* obj_renderer, ISimpleShader* obj_refShader
	, MeshEntityData* obj_mesh, CameraComponents* obj_matrices, SimpleShaderVariables* obj_shaderVars, SimpleShaderPixelVariables* obj_pixShaderVars
	, SimpleVertexShaderStruct* obj_vsStruct, SimplePixelShaderStruct* obj_PSStruct, MeshRenderVars* obj_vbib, entt::registry& registry, TimeData objtime, SkyShader* obj_SkyShader)
{
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// - Clear the render target and depth buffer (erases what's on the screen)
	// - Do this ONCE PER FRAME
	// - At the beginning of Draw (before drawing *anything*)
	obj_renderer->context->ClearRenderTargetView(obj_renderer->backBufferRTV, color);
	obj_renderer->context->ClearDepthStencilView(
		obj_renderer->depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Component info for search
	auto meshEntityComp = registry.view<MeshEntityData, MeshRenderVars>();

	// texture component id
	auto texture_comp = registry.view<TextureData>();

	// Search for each mesh entity
	for (auto entity : meshEntityComp)
	{	
		*obj_mesh = meshEntityComp.get<MeshEntityData>(entity); // transforms

		GameEntities::Rotate(0.0f, objtime.totalTime, 0.0f, &obj_mesh->rotation); // rotate the mesh

		*obj_vbib = meshEntityComp.get<MeshRenderVars>(entity); // get mesh info

		// Mesh movement and camera properties
		DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(obj_mesh->position.x, obj_mesh->position.y, obj_mesh->position.z);
		DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(obj_mesh->rotation.x);
		DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(obj_mesh->rotation.y);
		DirectX::XMMATRIX rotZ = DirectX::XMMatrixRotationZ(obj_mesh->rotation.z);
		DirectX::XMMATRIX sc = DirectX::XMMatrixScaling(obj_mesh->scale.x, obj_mesh->scale.y, obj_mesh->scale.z);

		// Need to implement rotation logic through mouse window event 
		DirectX::XMMATRIX total = sc * rotZ * rotY * rotX * trans; 

		XMStoreFloat4x4(&obj_mesh->worldMatrix, XMMatrixTranspose(total)); // transpose to match DX11 shader

		// Translated mesh vertices
		obj_refShader->SetData("world", &obj_mesh->worldMatrix, sizeof(float) * 16); 

		// Camera
		obj_refShader->SetData("view", &obj_matrices->viewMatrix, sizeof(float) * 16); 
		obj_refShader->SetData("projection", &obj_matrices->projMatrix, sizeof(float) * 16);

		// Copy vertex data to GPU
		obj_refShader->CopyAllBufferData(obj_renderer->context, obj_shaderVars->shaderValid, obj_shaderVars->constantBufferCount);
		obj_refShader->SetShaderAndCBs(obj_shaderVars->shaderValid, obj_renderer->context, obj_shaderVars->constantBufferCount, obj_vsStruct);

		// Allocate GPU memory for vb, ib and indices of the mesh
		// Still not displayed on screen. SwapChain() not called yet.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		obj_renderer->context->IASetVertexBuffers(0, 1, &obj_vbib->vb, &stride, &offset);
		obj_renderer->context->IASetIndexBuffer(obj_vbib->ib, DXGI_FORMAT_R32_UINT, 0);
		obj_renderer->context->DrawIndexed(obj_vbib->numIndices, 0, 0);		
	}
	
	// Send lighting and camera data to GPU
	obj_refShader->SetDataPixel("DirLightDirection", DirectX::XMFLOAT3(1, -1, 0), sizeof(float) * 3);
	obj_refShader->SetDataPixel("DirLightColor", DirectX::XMFLOAT3(0.75f, 0.75f, 0.75f), sizeof(float) * 3);
	obj_refShader->SetDataPixel("PointLightPosition", DirectX::XMFLOAT3(0, 5, 0), sizeof(float) * 3);
	obj_refShader->SetDataPixel("PointLightColor", DirectX::XMFLOAT3(0, 0, 1), sizeof(float) * 3);
	obj_refShader->SetDataPixel("CameraPosition", obj_matrices->position, sizeof(float) * 3);

	// Search for the texture properties
	// As it is 1 shader material, we could use 
	// different shaders or registers to transfer data
	for (auto entity : texture_comp)
	{
		TextureData& temp_obj = texture_comp.get<TextureData>(entity);

		// could render later because of sampler transparency
		DrawSky(registry, obj_renderer, texture_comp.get<TextureData>(entity).sampler, *obj_SkyShader, obj_matrices, &temp_obj); 

		obj_refShader->FindVariableBasic("BasicSampler", texture_comp.get<TextureData>(entity).sampler, obj_renderer->context);
		obj_refShader->FindVariableTexture("DiffuseTexture", texture_comp.get<TextureData>(entity).rustSRV, obj_renderer->context);
		obj_refShader->FindVariableTexture("SpecularMap", texture_comp.get<TextureData>(entity).specSRV, obj_renderer->context);

		// Store pixel shader data to GPU now
		obj_refShader->CopyAllBufferDataPixel(obj_renderer->context, obj_pixShaderVars->shaderValid, obj_pixShaderVars->constantBufferCount);
		obj_refShader->SetShaderAndCBsPixel(obj_pixShaderVars->shaderValid, obj_renderer->context, obj_pixShaderVars->constantBufferCount, obj_PSStruct);

	}
	// DO EVERYTHING ON GPU BEFORE THIS POINT
	obj_renderer->swapChain->Present(0, 0);
	
	// 1st paramenter is 1 as we have 1 render target
	// Second is the address of this target buffer
	// Third is depth, could be NULL too
	obj_renderer->context->OMSetRenderTargets(1, &obj_renderer->backBufferRTV, obj_renderer->depthStencilView);

	
}

void DXCore::DrawSky(entt::registry& registry, RendererMainVars* obj_renderer, ID3D11SamplerState* sampler, SkyShader& obj_SkyShader, CameraComponents* cam_Comp, TextureData* tex_Comp)
{
	auto skyVar = registry.view<MeshEntityDataSky, MeshRenderVarsSky>();

	auto mycompSky = registry.view<SimpleShaderPixelVariablesSky, SimpleShaderVertexVariablesSky, SkyVarsPixelShader, SkyVarsVertexShader>();

	entt::entity temp_Sky = registry.create();

	entt::entity temp_SkyTex = registry.create();

	for (auto entity : mycompSky)
	{
		temp_Sky = entity;
	}

	auto [ps_buf, vs_buf, ps_Sky, vs_Sky] = registry.get<SimpleShaderPixelVariablesSky, SimpleShaderVertexVariablesSky, SkyVarsPixelShader, SkyVarsVertexShader>(temp_Sky);

	auto sky_VarTex = registry.view<SkyVars>();

	for (auto entity : sky_VarTex)
	{
		temp_SkyTex = entity;
	}

	auto sky_GPUVars = registry.get<SkyVars>(temp_SkyTex);
	for (auto entity : skyVar)
	{
		auto [skyMesh, skyVars] = registry.get<MeshEntityDataSky, MeshRenderVarsSky>(entity);

		// Set buffers in the input assembler
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		obj_renderer->context->IASetVertexBuffers(0, 1, &skyVars.vb, &stride, &offset);
		obj_renderer->context->IASetIndexBuffer(skyVars.ib, DXGI_FORMAT_R32_UINT, 0);
		
		// Set up shaders
		obj_SkyShader.SetData("view", &cam_Comp->viewMatrix, sizeof(float) * 16);
		obj_SkyShader.SetData("projection", &cam_Comp->projMatrix, sizeof(float) * 16);
		obj_SkyShader.CopyAllBufferData(obj_renderer->context, vs_buf.shaderValid, vs_buf.constantBufferCount);
		obj_SkyShader.SetShaderAndCBs(vs_buf.shaderValid, obj_renderer->context, vs_buf.constantBufferCount, &vs_Sky);
		obj_SkyShader.FindVariableTexture("Sky", sky_GPUVars.skySRV, obj_renderer->context);
		obj_SkyShader.FindVariableBasic("BasicSampler", tex_Comp->sampler, obj_renderer->context);
		obj_SkyShader.SetShaderAndCBsPixel(ps_buf.shaderValid, obj_renderer->context, ps_buf.constantBufferCount, &ps_Sky);

		// Set up sky-specific render states
		obj_renderer->context->RSSetState(sky_GPUVars.skyRasterState);
		obj_renderer->context->OMSetDepthStencilState(sky_GPUVars.skyDepthState, 0);

		obj_renderer->context->DrawIndexed(skyVars.numIndices, 0, 0);

		// Reset states
		obj_renderer->context->RSSetState(0);
		obj_renderer->context->OMSetDepthStencilState(0, 0);

	}

}

//void DXCore::Update(float deltaTime, float totalTime, MeshEntityData* obj_mesh)
//{
//
//}


#pragma region Mouse Input



void DXCore::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future

	// Add any custom code here...


	prevMousePos.x = x;
	prevMousePos.y = y;
	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	//SetCapture(dxCoreWinRendererComps->hWnd);
}
void DXCore::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}
void DXCore::OnMouseMove(WPARAM buttonState, int x, int y)
{
	//// Add any custom code here...
	//// Check left mouse button
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera.Rotate(yDiff, xDiff, *registry2); // need to implement
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}
void DXCore::OnMouseWheel(float wheelDelta, int x, int y)
{
}

#pragma endregion



// --| NEED TO IMPLEMENT THIS LOGIC AS PART OF THE ECS |--
// When the window is resized, the underlying 
// buffers (textures) must also be resized to match.
// If we don't do this, the window size and our rendering
// resolution won't match up.  This can result in odd
// stretching/skewing.
// -------------------------------------------------------
//void DXCore::OnResize()
//{
//
//	// Release existing DirectX views and buffers
//	if (dxCoreWinRendererComps->depthStencilView) { dxCoreWinRendererComps->depthStencilView->Release(); }
//	if (dxCoreWinRendererComps->backBufferRTV) { dxCoreWinRendererComps->backBufferRTV->Release(); }
//
//	// Resize the underlying swap chain buffers
//	dxCoreWinRendererComps->swapChain->ResizeBuffers(
//		1,
//		dxCoreWinRendererComps->width,
//		dxCoreWinRendererComps->height,
//		DXGI_FORMAT_R8G8B8A8_UNORM,
//		0);
//
//	// Recreate the render target view for the back buffer
//	// texture, then release our local texture reference
//	ID3D11Texture2D* backBufferTexture;
//	dxCoreWinRendererComps->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture));
//	dxCoreWinRendererComps->device->CreateRenderTargetView(backBufferTexture, 0, &dxCoreWinRendererComps->backBufferRTV);
//	backBufferTexture->Release();
//
//	// Set up the description of the texture to use for the depth buffer
//	D3D11_TEXTURE2D_DESC depthStencilDesc;
//	depthStencilDesc.Width				= dxCoreWinRendererComps->width;
//	depthStencilDesc.Height				= dxCoreWinRendererComps->height;
//	depthStencilDesc.MipLevels			= 1;
//	depthStencilDesc.ArraySize			= 1;
//	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
//	depthStencilDesc.Usage				= D3D11_USAGE_DEFAULT;
//	depthStencilDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
//	depthStencilDesc.CPUAccessFlags		= 0;
//	depthStencilDesc.MiscFlags			= 0;
//	depthStencilDesc.SampleDesc.Count	= 1;
//	depthStencilDesc.SampleDesc.Quality = 0;
//
//	// Create the depth buffer and its view, then 
//	// release our reference to the texture
//	ID3D11Texture2D* depthBufferTexture;
//	dxCoreWinRendererComps->device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
//	dxCoreWinRendererComps->device->CreateDepthStencilView(depthBufferTexture, 0, &dxCoreWinRendererComps->depthStencilView);
//	depthBufferTexture->Release();
//
//	// Bind the views to the pipeline, so rendering properly 
//	// uses their underlying textures
//	dxCoreWinRendererComps->context->OMSetRenderTargets(1, &dxCoreWinRendererComps->backBufferRTV, dxCoreWinRendererComps->depthStencilView);
//
//	// Lastly, set up a viewport so we render into
//	// to correct portion of the window
//	D3D11_VIEWPORT viewport = {};
//	viewport.TopLeftX = 0;
//	viewport.TopLeftY = 0;
//	viewport.Width = (float)dxCoreWinRendererComps->width;
//	viewport.Height = (float)dxCoreWinRendererComps->height;
//	viewport.MinDepth = 0.0f;
//	viewport.MaxDepth = 1.0f;
//	dxCoreWinRendererComps->context->RSSetViewports(1, &viewport);
//}


// --------------------------------------------------------
// Uses high resolution time stamps to get very accurate
// timing information, and calculates useful time stats
// --------------------------------------------------------
void DXCore::UpdateTimer()
{
	// Grab the current time
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	obj_time.currentTime = now;

	// Calculate delta time and clamp to zero
	//  - Could go negative if CPU goes into power save mode 
	//    or the process itself gets moved to another core
	obj_time.deltaTime = max((float)((obj_time.currentTime - obj_time.previousTime) * obj_time.perfCounterSeconds), 0.0f);

	// Calculate the total time from start to now
	obj_time.totalTime = (float)((obj_time.currentTime - obj_time.startTime) * obj_time.perfCounterSeconds);

	// Save current time for next frame
	obj_time.previousTime = obj_time.currentTime;
}


//
// --------------------------------------------------------
// Updates the window's title bar with several stats once
// per second, including:
//  - The window's width & height
//  - The current FPS and ms/frame
//  - The version of DirectX actually being used (usually 11)
// --------------------------------------------------------
void DXCore::UpdateTitleBarStats(D3D_FEATURE_LEVEL dxFeatureLevelPar, FPSData& obj_FPS, std::string& titleBarText, entt::registry& registry)
{
	obj_FPS.fpsFrameCount++;

	// Only calc FPS and update title bar once per second
	float timeDiff = obj_time.totalTime - obj_FPS.fpsTimeElapsed;
	if (timeDiff < 1.0f)
		return;

	// How long did each frame take?  (Approx)
	float mspf = 1000.0f / (float)obj_FPS.fpsFrameCount;

	// Quick and dirty title bar text (mostly for debugging)
	std::ostringstream output;
	output.precision(6);
	output << titleBarText <<
		"    Width: "		<< 1280 <<
		"    Height: "		<< 720 <<
		"    FPS: "			<< obj_FPS.fpsFrameCount <<
		"    Frame Time: "	<< mspf << "ms";

	// Append the version of DirectX the app is using
	switch (dxFeatureLevelPar)
	{
	case D3D_FEATURE_LEVEL_11_1: output << "    DX 11.1"; break;
	case D3D_FEATURE_LEVEL_11_0: output << "    DX 11.0"; break;
	case D3D_FEATURE_LEVEL_10_1: output << "    DX 10.1"; break;
	case D3D_FEATURE_LEVEL_10_0: output << "    DX 10.0"; break;
	case D3D_FEATURE_LEVEL_9_3:  output << "    DX 9.3";  break;
	case D3D_FEATURE_LEVEL_9_2:  output << "    DX 9.2";  break;
	case D3D_FEATURE_LEVEL_9_1:  output << "    DX 9.1";  break;
	default:                     output << "    DX ???";  break;
	}

	auto mainWinComp = registry.view<RenderWindow, RendererMainVars, RenderWindowDimensions>();

	for (auto entity : mainWinComp)
	{
		auto [mycomp, mycomp2, mycomp3] = mainWinComp.get<RenderWindow, RendererMainVars, RenderWindowDimensions>(entity);

		// Actually update the title bar and reset fps data
		SetWindowText(mycomp.hWnd, output.str().c_str());
	}
	obj_FPS.fpsFrameCount = 0;
	obj_FPS.fpsTimeElapsed += 1.0f;
}

// -----------------------------------------------------------------
// Handles messages that are sent to our window by the
// operating system.  Ignoring these messages would cause
// our program to hang and Windows would think it was unresponsive
// -----------------------------------------------------------------
LRESULT DXCore::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//auto mainWinComp = dxRegistry.view<RenderWindowDimensions, RendererMainVars>();
	// Check the incoming message and handle any we care about
	switch (uMsg)
	{	
	// This is the message that signifies the window closing
	case WM_DESTROY:
		PostQuitMessage(0);	// Send a quit message to our own program
		return 0;

	// Prevent beeping when we "alt-enter" into fullscreen
	case WM_MENUCHAR: 
		return MAKELRESULT(0, MNC_CLOSE);

	// Prevent the overall window from becoming too small
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	// Sent when the window size changes
	case WM_SIZE:
		// Don't adjust anything when minimizing,
		// since we end up with a width/height of zero
		// and that doesn't play well with DirectX
		if (wParam == SIZE_MINIMIZED)
			return 0;

		//dxCoreWinRendererComps->width = LOWORD(lParam);
		//dxCoreWinRendererComps->height = HIWORD(lParam);
		////// If DX is initialized, resize 
		////// our required buffers
		//if (dxCoreWinRendererComps->device)
		//	OnResize();

		return 0;

	// Mouse button being pressed (while the cursor is currently over our window)
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	// Mouse button being released (while the cursor is currently over our window)
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	// Cursor moves over the window (or outside, while we're currently capturing it)
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	// Mouse wheel is scrolled
	case WM_MOUSEWHEEL:
		OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	//case WM_KEYDOWN:
	//	SpawnMeshes();
	//	return 0;
	}

	// Let Windows handle any messages we're not touching
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

