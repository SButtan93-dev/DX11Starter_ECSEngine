#include "BasicShader.h"
unsigned int constanBufferCount1;


BasicShader::BasicShader()
{

}


BasicShader::~BasicShader()
{

	//pixel
	for (unsigned int i = 0; i < constanBufferCount1; i++)
	{
		//s_shaderVecs.constantBuffers[i].ConstantBuffer->Release();
		s_shaderVecsPixel.constantBuffers[i].ConstantBuffer = s_shaderVecsVertex.constantBuffers[i].ConstantBuffer;
		s_shaderVecsPixel.constantBuffers[i].ConstantBuffer->Release();
		delete[] s_shaderVecsPixel.constantBuffers[i].LocalDataBuffer;
	}

	if (s_shaderVecsPixel.constantBuffers)
	{
		delete[] s_shaderVecsPixel.constantBuffers;
	}

	for (unsigned int i = 0; i < s_shaderVecsPixel.shaderResourceViews.size(); i++)
		delete s_shaderVecsPixel.shaderResourceViews[i];

	for (unsigned int i = 0; i < s_shaderVecsPixel.samplerStates.size(); i++)
		delete s_shaderVecsPixel.samplerStates[i];

	// Clean up tables
	s_shaderVecsPixel.varTable.clear();
	s_shaderVecsPixel.cbTable.clear();
	s_shaderVecsPixel.samplerTable.clear();
	s_shaderVecsPixel.textureTable.clear();
	
	// vertex
	// Handle constant buffers and local data buffers
	for (unsigned int i = 0; i < constanBufferCount1; i++)
	{
		s_shaderVecsVertex.constantBuffers[i].ConstantBuffer->Release();
		delete[] s_shaderVecsVertex.constantBuffers[i].LocalDataBuffer;
	}

	if (s_shaderVecsVertex.constantBuffers)
	{
		delete[] s_shaderVecsVertex.constantBuffers;
		constanBufferCount1 = 0;
	}

	for (unsigned int i = 0; i < s_shaderVecsVertex.shaderResourceViews.size(); i++)
		delete s_shaderVecsVertex.shaderResourceViews[i];

	for (unsigned int i = 0; i < s_shaderVecsVertex.samplerStates.size(); i++)
		delete s_shaderVecsVertex.samplerStates[i];

	// Clean up tables
	s_shaderVecsVertex.varTable.clear();
	s_shaderVecsVertex.cbTable.clear();
	s_shaderVecsVertex.samplerTable.clear();
	s_shaderVecsVertex.textureTable.clear();

}

// Initialize vertex and pixel shader pipeline components to an entity
entt::registry& BasicShader::InitShaderBegin(entt::registry & registry)
{
	auto s_initialEntity = registry.create();

	// shaderFile - A "wide string" specifying the compiled shader to load
	ShaderStrings shaderStringsObj;
	shaderStringsObj.pixelShaderString = L"PixelShader.cso";
	shaderStringsObj.vertexShaderString = L"VertexShader.cso";

	ShaderStringsSky shaderStringsObjSky;
	shaderStringsObjSky.SkyPixelShaderString = L"SkyPS.cso";
	shaderStringsObjSky.SkyVertexShaderString = L"SkyVS.cso";

	registry.emplace<InputLayoutVertexShader>(s_initialEntity, false, nullptr, nullptr);
	registry.emplace<PixelShader>(s_initialEntity, nullptr);
	registry.emplace<VertexShaderVars>(s_initialEntity, false, nullptr, nullptr, (unsigned int)0);
	registry.emplace<PixelShaderVars>(s_initialEntity, false, nullptr,	
		nullptr, (unsigned int)0);

	// Sky stuff!
	registry.emplace<SkyVarsVertexShader>(s_initialEntity, false, nullptr, nullptr); //vs
	registry.emplace<SkyVarsPixelShader>(s_initialEntity, nullptr); // ps
	registry.emplace<SkyVars>(s_initialEntity, nullptr, nullptr, nullptr); // srv, raster & depth
	registry.emplace<SkyVS_Vars>(s_initialEntity, false, nullptr, nullptr, (unsigned int)0);
	registry.emplace<SkyPS_Vars>(s_initialEntity, false, nullptr, nullptr, (unsigned int)0); 

	registry.emplace<ShaderStrings>(s_initialEntity, shaderStringsObj.vertexShaderString, shaderStringsObj.pixelShaderString);
	registry.emplace<ShaderStringsSky>(s_initialEntity, shaderStringsObjSky.SkyVertexShaderString, shaderStringsObjSky.SkyPixelShaderString);
	
	return registry;
}


// Set view and projection matrices for the camera
void BasicShader::CreateMatrices(entt::registry& registry)
{
	// Get camera component id
	auto s_mainShaderComp = registry.view<CameraComponents>();

	// Pass local to registry
	DirectX::XMFLOAT4 rotation;
	DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionIdentity());

	// Check for the camera entity using the component id
	for (auto entity : s_mainShaderComp)
	{
		auto &c_VertexShaderVars =
			s_mainShaderComp.get<CameraComponents>(entity);
			
		DirectX::XMFLOAT4X4 view = c_VertexShaderVars.viewMatrix;
		DirectX::XMFLOAT4X4 projection = c_VertexShaderVars.projMatrix;
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
			0.25f * DirectX::XM_PI,		// Field of View Angle
			(float)(1280 / 720),		// Aspect ratio
			0.1f,				// Near clip plane distance
			1000.0f);			// Far clip plane distance
		XMStoreFloat4x4(&projection, XMMatrixTranspose(P)); // Transpose for HLSL!

		// Set position value manually, could be set by a local variable too..
		registry.replace<CameraComponents>(entity, view, projection, DirectX::XMFLOAT3(0.0f,0.0f, 0.0f), DirectX::XMFLOAT3(10.0f, 0.0f,-30.0f), rotation, 0.0f, 0.0f);
	}
}


// --------------------------------------------------------
// Creates the DirectX vertex shader
//
// shaderBlob - The shader's compiled code
//
// Returns true if shader is created correctly, false otherwise
// --------------------------------------------------------
bool BasicShader::CreateVertexShader(entt::registry& registry)
{

	auto s_mainShaderComp = registry.view<RendererMainVars>();
	auto mycomp = registry.view<VertexShaderVars, InputLayoutVertexShader>();

	entt::entity entity1;

	entt::entity entity2;

	for (auto entity : s_mainShaderComp)
	{
		entity1 = entity;	
	}

	auto c_rendererMainVars = registry.get<RendererMainVars>(entity1);

	for (auto entity : mycomp)
	{
		entity2 = entity;	
	}

	auto [c_VertexShaderVars, c_InputLayoutVertexShader] = registry.get<VertexShaderVars, InputLayoutVertexShader>(entity2);

		// Create the shader from the blob
	HRESULT result = c_rendererMainVars.device->CreateVertexShader(
		c_VertexShaderVars.shaderBlob->GetBufferPointer(),
		c_VertexShaderVars.shaderBlob->GetBufferSize(),
		0,
		&c_InputLayoutVertexShader.shader);

	// Did the creation work?
	if (result != S_OK)
		return false;

	// Do we already have an input layout?
	if (c_InputLayoutVertexShader.inputLayout)
		return true;

	// Vertex shader was created successfully, so we now use the
	// shader code to re-reflect and create an input layout that 
	// matches what the vertex shader expects.  Code adapted from:
	// https://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/

	// Reflect shader info
	ID3D11ShaderReflection* refl;
	D3DReflect(
		c_VertexShaderVars.shaderBlob->GetBufferPointer(),
		c_VertexShaderVars.shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&refl);

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		refl->GetInputParameterDesc(i, &paramDesc);

		// Check the semantic name for "_PER_INSTANCE"
		std::string perInstanceStr = "_PER_INSTANCE";
		std::string sem = paramDesc.SemanticName;
		int lenDiff = (int)sem.size() - (int)perInstanceStr.size();
		bool isPerInstance =
			lenDiff >= 0 &&
			sem.compare(lenDiff, perInstanceStr.size(), perInstanceStr) == 0;

		// Fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Replace anything affected by "per instance" data
		if (isPerInstance)
		{
			elementDesc.InputSlot = 1; // Assume per instance data comes from another input slot!
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;

			c_InputLayoutVertexShader.perInstanceCompatible = true;
		}

		// Determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		// Save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = c_rendererMainVars.device->CreateInputLayout(
		&inputLayoutDesc[0],
		(unsigned int)inputLayoutDesc.size(),
		c_VertexShaderVars.shaderBlob->GetBufferPointer(),
		c_VertexShaderVars.shaderBlob->GetBufferSize(),
		&c_InputLayoutVertexShader.inputLayout);

	registry.replace<RendererMainVars>(entity1, c_rendererMainVars.swapChain, c_rendererMainVars.device,
		c_rendererMainVars.context, c_rendererMainVars.backBufferRTV, c_rendererMainVars.depthStencilView);

	registry.replace<InputLayoutVertexShader>(entity2, c_InputLayoutVertexShader.perInstanceCompatible, c_InputLayoutVertexShader.inputLayout,
		c_InputLayoutVertexShader.shader);

	// All done, clean up
	refl->Release();

	return true;
}




// --------------------------------------------------------
// Creates the DirectX pixel shader
//
// shaderBlob - The shader's compiled code
//
// Returns true if shader is created correctly, false otherwise
// --------------------------------------------------------
bool BasicShader::CreatePixelShader(entt::registry & registry)
{
	entt::entity gpu_main;

	entt::entity ps_entity;
	auto mycomp = registry.view<RendererMainVars>();

	auto mycomp2 = registry.view<PixelShader, PixelShaderVars>();

	for (auto entity : mycomp)
	{
		gpu_main = entity;
	}

	for (auto entity : mycomp2)
	{
		ps_entity = entity;
	}

	auto &c_win = mycomp.get<RendererMainVars>(gpu_main);

	auto [c_pixStruct, c_pixVars] = mycomp2.get<PixelShader, PixelShaderVars>(ps_entity);
	// Create the shader from the blob
	HRESULT result = c_win.device->CreatePixelShader(
		c_pixVars.shaderBlob->GetBufferPointer(),
		c_pixVars.shaderBlob->GetBufferSize(),
		0,
		&c_pixStruct.shader);

	registry.replace<PixelShader>(ps_entity, c_pixStruct.shader);
	// Check the result
	return (result == S_OK);
}

// -------------------------------
// Part of creating vertex shader
// -------------------------------
entt::registry& BasicShader::ReadFileToBlobVertex(entt::registry & registry)
{
	auto mainShaderComp = registry.view<VertexShaderVars, ShaderStrings>();

	for (auto entity : mainShaderComp)
	{
		auto[mycomp, mycomp2] = mainShaderComp.get<VertexShaderVars, ShaderStrings>(entity);
		// Load the shader to a blob and ensure it worked
		D3DReadFileToBlob(mycomp2.vertexShaderString, &mycomp.shaderBlob); // also returns HRESULT

		registry.replace<VertexShaderVars>(entity, mycomp.shaderValid, mycomp.shaderBlob,
			mycomp.ConstantBuffer, mycomp.constantBufferCount);
	}
	return registry;
}

// ------------------------------
// Part of creating pixel shader
// ------------------------------
entt::registry& BasicShader::ReadFileToBlobPixel(entt::registry & registry)
{
	auto mainShaderComp = registry.view<PixelShaderVars>();

	LPCWSTR abc = L"PixelShader.cso"; // test
	for (auto entity : mainShaderComp)
	{
		auto mycomp = mainShaderComp.get<PixelShaderVars>(entity);
		// Load the shader to a blob and ensure it worked
		HRESULT hr = D3DReadFileToBlob(abc, &mycomp.shaderBlob);
		if (hr != S_OK)
		{
			//return false;
		}

		registry.replace<PixelShaderVars>(entity, mycomp.shaderValid, mycomp.shaderBlob,
			mycomp.ConstantBuffer, mycomp.constantBufferCount);
	}
	return registry;
}




// -----------------------------------------------------------------------------------------
// Loads the specified vertex shader and builds the variable table using shader reflection.  
//
// shaderFile - A "wide string" specifying the compiled shader to load
// -----------------------------------------------------------------------------------------
void BasicShader::LoadVertexShaderFile(entt::registry &registry)
{

	bool checkLoadShader = CreateVertexShader(ReadFileToBlobVertex(registry));

	auto d_component = registry.view<RendererMainVars>();

	for (auto entity : d_component)
	{
		auto& get_component = d_component.get<RendererMainVars>(entity);

		auto mainShaderComp = registry.view<VertexShaderVars>();

		for (auto entity : mainShaderComp)
		{
			auto mycomp = mainShaderComp.get<VertexShaderVars>(entity);

			// Create the shader - Calls an overloaded version of this abstract
			// method in the appropriate child class

			// Set up shader reflection to get information about
			// this shader and its variables,  buffers, etc.
			ID3D11ShaderReflection* refl;
			D3DReflect(
				mycomp.shaderBlob->GetBufferPointer(),
				mycomp.shaderBlob->GetBufferSize(),
				IID_ID3D11ShaderReflection,
				(void**)&refl);

			// Get the description of the shader
			D3D11_SHADER_DESC shaderDesc;
			refl->GetDesc(&shaderDesc);

			// Create resource arrays
			mycomp.constantBufferCount = shaderDesc.ConstantBuffers;
			s_shaderVecsVertex.constantBuffers = new ConstantBufferInfo[mycomp.constantBufferCount];

			// Handle bound resources (like shaders and samplers)
			unsigned int resourceCount = shaderDesc.BoundResources;
			for (unsigned int r = 0; r < resourceCount; r++)
			{
				// Get this resource's description
				D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
				refl->GetResourceBindingDesc(r, &resourceDesc);

				// Check the type
				switch (resourceDesc.Type)
				{
				case D3D_SIT_TEXTURE: // A texture resource
				{
					// Create the SRV wrapper
					BasicSRV* srv = new BasicSRV();
					srv->BindIndex = resourceDesc.BindPoint;				// Shader bind point
					srv->Index = (unsigned int)s_shaderVecsVertex.shaderResourceViews.size();	// Raw index

					s_shaderVecsVertex.textureTable.insert(std::pair<std::string, BasicSRV*>(resourceDesc.Name, srv));
					s_shaderVecsVertex.shaderResourceViews.push_back(srv);
				}
				break;

				case D3D_SIT_SAMPLER: // A sampler resource
				{
					//Create the sampler wrapper
					BasicSampler* samp = new BasicSampler();
					samp->BindIndex = resourceDesc.BindPoint;			// Shader bind point
					samp->Index = (unsigned int)s_shaderVecsVertex.samplerStates.size();	// Raw index

					s_shaderVecsVertex.samplerTable.insert(std::pair<std::string, BasicSampler*>(resourceDesc.Name, samp));
					s_shaderVecsVertex.samplerStates.push_back(samp);
				}
				break;
				}
			}

			// Loop through all constant buffers
			for (unsigned int b = 0; b < mycomp.constantBufferCount; b++)
			{
				// Get this buffer
				ID3D11ShaderReflectionConstantBuffer* cb =
					refl->GetConstantBufferByIndex(b);

				// Get the description of this buffer
				D3D11_SHADER_BUFFER_DESC bufferDesc;
				cb->GetDesc(&bufferDesc);

				// Get the description of the resource binding, so
				// we know exactly how it's bound in the shader
				D3D11_SHADER_INPUT_BIND_DESC bindDesc;
				refl->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

				// Set up the buffer and put its pointer in the table
				s_shaderVecsVertex.constantBuffers[b].BindIndex = bindDesc.BindPoint;
				s_shaderVecsVertex.constantBuffers[b].Name = bufferDesc.Name;
				s_shaderVecsVertex.cbTable.insert(std::pair<std::string, ConstantBufferInfo*>(bufferDesc.Name, &s_shaderVecsVertex.constantBuffers[b]));

				// Create this constant buffer
				D3D11_BUFFER_DESC newBuffDesc;
				newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
				newBuffDesc.ByteWidth = bufferDesc.Size;
				newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				newBuffDesc.CPUAccessFlags = 0;
				newBuffDesc.MiscFlags = 0;
				newBuffDesc.StructureByteStride = 0;
				get_component.device->CreateBuffer(&newBuffDesc, 0, &s_shaderVecsVertex.constantBuffers[b].ConstantBuffer);

				// Set up the data buffer for this constant buffer
				s_shaderVecsVertex.constantBuffers[b].Size = bufferDesc.Size;
				s_shaderVecsVertex.constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
				ZeroMemory(s_shaderVecsVertex.constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

				// Loop through all variables in this buffer
				for (unsigned int v = 0; v < bufferDesc.Variables; v++)
				{
					// Get this variable
					ID3D11ShaderReflectionVariable* var =
						cb->GetVariableByIndex(v);

					// Get the description of the variable and its type
					D3D11_SHADER_VARIABLE_DESC varDesc;
					var->GetDesc(&varDesc);

					// Create the variable struct
					ShaderVariableInfo varStruct;
					varStruct.ConstantBufferIndex = b;
					varStruct.ByteOffset = varDesc.StartOffset;
					varStruct.Size = varDesc.Size;

					// Get a string version
					std::string varName(varDesc.Name);

					// Add this variable to the table and the constant buffer
					s_shaderVecsVertex.varTable.insert(std::pair<std::string, ShaderVariableInfo>(varName, varStruct));
					s_shaderVecsVertex.constantBuffers[b].Variables.push_back(varStruct);
				}
			}

			registry.replace<VertexShaderVars>(entity, mycomp.shaderValid, mycomp.shaderBlob, mycomp.ConstantBuffer, mycomp.constantBufferCount);

			// All set
			refl->Release();
		}
	}
}


// -----------------------------------------------------------------------------------
// Loads the specified pixel shader and builds the variable table using shader reflection.  
//
// shaderFile - A "wide string" specifying the compiled shader to load
// -----------------------------------------------------------------------------------
void BasicShader::LoadPixelShaderFile(entt::registry &registry)
{

	bool checkLoadShader = CreatePixelShader(ReadFileToBlobPixel(registry));

	auto d_component = registry.view<RendererMainVars>();

	for (auto entity : d_component)
	{
		RendererMainVars& renderer_component = d_component.get<RendererMainVars>(entity);

		auto mainShaderComp = registry.view<PixelShaderVars>();

		for (auto entity : mainShaderComp)
		{
			auto mycomp = mainShaderComp.get<PixelShaderVars>(entity);

			// Set up shader reflection to get information about
			// this shader and its variables,  buffers, etc.
			ID3D11ShaderReflection* refl;
			D3DReflect(
				mycomp.shaderBlob->GetBufferPointer(),
				mycomp.shaderBlob->GetBufferSize(),
				IID_ID3D11ShaderReflection,
				(void**)&refl);

			// Get the description of the shader
			D3D11_SHADER_DESC shaderDesc;
			refl->GetDesc(&shaderDesc);

			// Create resource arrays
			mycomp.constantBufferCount = shaderDesc.ConstantBuffers;
			s_shaderVecsPixel.constantBuffers = new ConstantBufferInfo[mycomp.constantBufferCount];

			// Handle bound resources (like shaders and samplers)
			unsigned int resourceCount = shaderDesc.BoundResources;
			for (unsigned int r = 0; r < resourceCount; r++)
			{
				// Get this resource's description
				D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
				refl->GetResourceBindingDesc(r, &resourceDesc);

				// Check the type
				switch (resourceDesc.Type)
				{
				case D3D_SIT_TEXTURE: // A texture resource
				{
					// Create the SRV wrapper
					BasicSRV* srv = new BasicSRV();
					srv->BindIndex = resourceDesc.BindPoint;				// Shader bind point
					srv->Index = (unsigned int)s_shaderVecsPixel.shaderResourceViews.size();	// Raw index

					s_shaderVecsPixel.textureTable.insert(std::pair<std::string, BasicSRV*>(resourceDesc.Name, srv));
					s_shaderVecsPixel.shaderResourceViews.push_back(srv);
				}
				break;

				case D3D_SIT_SAMPLER: // A sampler resource
				{
					//Create the sampler wrapper
					BasicSampler* samp = new BasicSampler();
					samp->BindIndex = resourceDesc.BindPoint;			// Shader bind point
					samp->Index = (unsigned int)s_shaderVecsPixel.samplerStates.size();	// Raw index

					s_shaderVecsPixel.samplerTable.insert(std::pair<std::string, BasicSampler*>(resourceDesc.Name, samp));
					s_shaderVecsPixel.samplerStates.push_back(samp);
				}
				break;
				}
			}

			// Loop through all constant buffers
			for (unsigned int b = 0; b < mycomp.constantBufferCount; b++)
			{
				// Get this buffer
				ID3D11ShaderReflectionConstantBuffer* cb =
					refl->GetConstantBufferByIndex(b);

				// Get the description of this buffer
				D3D11_SHADER_BUFFER_DESC bufferDesc;
				cb->GetDesc(&bufferDesc);

				// Get the description of the resource binding, so
				// we know exactly how it's bound in the shader
				D3D11_SHADER_INPUT_BIND_DESC bindDesc;
				refl->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

				// Set up the buffer and put its pointer in the table
				s_shaderVecsPixel.constantBuffers[b].BindIndex = bindDesc.BindPoint;
				s_shaderVecsPixel.constantBuffers[b].Name = bufferDesc.Name;
				s_shaderVecsPixel.cbTable.insert(std::pair<std::string, ConstantBufferInfo*>(bufferDesc.Name, &s_shaderVecsPixel.constantBuffers[b]));

				// Create this constant buffer
				D3D11_BUFFER_DESC newBuffDesc;
				newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
				newBuffDesc.ByteWidth = bufferDesc.Size;
				newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				newBuffDesc.CPUAccessFlags = 0;
				newBuffDesc.MiscFlags = 0;
				newBuffDesc.StructureByteStride = 0;
				renderer_component.device->CreateBuffer(&newBuffDesc, 0, &s_shaderVecsPixel.constantBuffers[b].ConstantBuffer);

				// Set up the data buffer for this constant buffer
				s_shaderVecsPixel.constantBuffers[b].Size = bufferDesc.Size;
				s_shaderVecsPixel.constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
				ZeroMemory(s_shaderVecsPixel.constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

				// Loop through all variables in this buffer
				for (unsigned int v = 0; v < bufferDesc.Variables; v++)
				{
					// Get this variable
					ID3D11ShaderReflectionVariable* var =
						cb->GetVariableByIndex(v);

					// Get the description of the variable and its type
					D3D11_SHADER_VARIABLE_DESC varDesc;
					var->GetDesc(&varDesc);

					// Create the variable struct
					ShaderVariableInfo varStruct;
					varStruct.ConstantBufferIndex = b;
					varStruct.ByteOffset = varDesc.StartOffset;
					varStruct.Size = varDesc.Size;

					// Get a string version
					std::string varName(varDesc.Name);

					// Add this variable to the table and the constant buffer
					s_shaderVecsPixel.varTable.insert(std::pair<std::string, ShaderVariableInfo>(varName, varStruct));
					s_shaderVecsPixel.constantBuffers[b].Variables.push_back(varStruct);
				}
			}

			registry.replace<PixelShaderVars>(entity, mycomp.shaderValid, mycomp.shaderBlob, mycomp.ConstantBuffer, mycomp.constantBufferCount);

			// All set
			refl->Release();
		}
	}
}


// ---------------------------------------------------------------------
// Sets variable by name with arbitrary data of the specified size in vs
// name - The name of the shader variable
// data - The data to set in the buffer
// size - The size of the data (this must match the variable's size)
// ---------------------------------------------------------------------
void BasicShader::SetDataVertex(std::string name, const void* data, unsigned int size)
{
	// Look for the variable and verify
	ShaderVariableInfo* var = FindVariableVertex(name, size);

	// Set the data in the local data buffer
	memcpy(
		s_shaderVecsVertex.constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
		data,
		size);
}


// -------------------------------------------------------------------
// Sets variable by name with arbitrary data of the specified size in ps
// name - The name of the shader variable
// data - The data to set in the buffer
// size - The size of the data (this must match the variable's size)
// ------------------------------------------------------------------
void BasicShader::SetDataPixel(std::string name, const DirectX::XMFLOAT3 data, unsigned int size)
{
	// Look for the variable and verify
	ShaderVariableInfo* var = FindVariablePixel(name, size);

	// Set the data in the local data buffer
	memcpy(
		s_shaderVecsPixel.constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
		&data,
		size);
}


 //--------------------------------------------------------
 //-Helper for looking up a variable by vertex shader name and also
 //verifying that it is the requested size
 //-Search in global vs variable for now, must be retrieved from registry
 //-name - the name of the variable to look for
 //-size - the size of the variable (for verification), or -1 to bypass
 //-return vertex shader variable from the table
 //--------------------------------------------------------
ShaderVariableInfo* BasicShader::FindVariableVertex(std::string name, int size)
{
	// Look for the key
	std::unordered_map<std::string, ShaderVariableInfo>::iterator result =
		s_shaderVecsVertex.varTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsVertex.varTable.end())
		return 0;

	// Grab the result from the iterator
	ShaderVariableInfo* var = &(result->second);

	// Is the data size correct ?
	if (size > 0 && var->Size != size)
		return 0;

	// Success
	return var;
}


//--------------------------------------------------------
//-Helper for looking up a variable by pixel shader name and also
//verifying that it is the requested size
//-Search in global ps variable for now, must be retrieved from registry
//-name - the name of the variable to look for
//-size - the size of the variable (for verification), or -1 to bypass
//-return pixel shader variable from the table
//--------------------------------------------------------
ShaderVariableInfo * BasicShader::FindVariablePixel(std::string name, int size)
{
	// Look for the key
	std::unordered_map<std::string, ShaderVariableInfo>::iterator result =
		s_shaderVecsPixel.varTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixel.varTable.end())
		return 0;

	// Grab the result from the iterator
	ShaderVariableInfo* var = &(result->second);

	// Is the data size correct ?
	if (size > 0 && var->Size != size)
		return 0;

	// Success
	return var;
}


void BasicShader::FindVariableBasic(std::string name, ID3D11SamplerState* samplerState, ID3D11DeviceContext* deviceContext)
{
	// Look for the key
	std::unordered_map<std::string, BasicSampler*>::iterator result =
		s_shaderVecsPixel.samplerTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixel.samplerTable.end())
		return;

	const BasicSampler* sampInfo = result->second;

	if (sampInfo == 0)
		return;

	// Set the shader resource view
	deviceContext->PSSetSamplers(sampInfo->BindIndex, 1, &samplerState);
}


void BasicShader::FindVariableTexture(std::string name, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* deviceContext)
{
	// Look for the key
	std::unordered_map<std::string, BasicSRV*>::iterator result =
		s_shaderVecsPixel.textureTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixel.textureTable.end())
		return;

	const BasicSRV* srvInfo = result->second;

	if (srvInfo == 0)
		return;

	// Set the shader resource view
	deviceContext->PSSetShaderResources(srvInfo->BindIndex, 1, &srv);
}


// --------------------------------------------------------
// Copies the relevant data to the all of this 
// shader's constant buffers.  To just copy one
// buffer, use CopyBufferData()
// --------------------------------------------------------
void BasicShader::CopyAllBufferDataVertex(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount)
{
	// Ensure the shader is valid
	//if (!shaderValid) return;

	// Loop through the constant buffers and copy all data
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Copy the entire local data buffer
		deviceContext->UpdateSubresource(
		s_shaderVecsVertex.constantBuffers[i].ConstantBuffer, 0, 0,
			s_shaderVecsVertex.constantBuffers[i].LocalDataBuffer, 0, 0);
	}
}


void BasicShader::CopyAllBufferDataPixel(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount)
{
	// Ensure the shader is valid
	// if (!shaderValid) return;

	// Loop through the constant buffers and copy all data
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Copy the entire local data buffer
		deviceContext->UpdateSubresource(
			s_shaderVecsPixel.constantBuffers[i].ConstantBuffer, 0, 0,
			s_shaderVecsPixel.constantBuffers[i].LocalDataBuffer, 0, 0);
	}
}


// --------------------------------------------------------
// Sets the vertex shader, input layout and constant buffers
// for future DirectX drawing
// --------------------------------------------------------
void BasicShader::SetShaderAndCBsVertex(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, InputLayoutVertexShader* obj_vsStruct)
{
	// Is shader valid?
	//if (!shaderValid) return;

	// Set the shader and input layout
	deviceContext->IASetInputLayout(obj_vsStruct->inputLayout);
	deviceContext->VSSetShader(obj_vsStruct->shader, 0, 0);


	// Set the constant buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		deviceContext->VSSetConstantBuffers(
		s_shaderVecsVertex.constantBuffers[i].BindIndex,
			1,
			&s_shaderVecsVertex.constantBuffers[i].ConstantBuffer);
	}

	constanBufferCount1 = constantBufferCount;
}


// --------------------------------------------------------
// Sets the vertex shader, input layout and constant buffers
// for future DirectX drawing
// --------------------------------------------------------
void BasicShader::SetShaderAndCBsPixel(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, PixelShader* obj_psStruct)
{
	// Is shader valid?
	//if (!shaderValid) return;

	// Set the shader
	deviceContext->PSSetShader(obj_psStruct->shader, 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		deviceContext->PSSetConstantBuffers(
		s_shaderVecsPixel.constantBuffers[i].BindIndex,
			1,
			&s_shaderVecsPixel.constantBuffers[i].ConstantBuffer);
	}
}