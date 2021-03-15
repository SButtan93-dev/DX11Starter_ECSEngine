#include "SkyShader.h"
#include "DDSTextureLoader.h"
unsigned int constanBufferCount2;

SkyShader::SkyShader()
{
}

SkyShader::~SkyShader()
{
	//pixel
	for (unsigned int i = 0; i < constanBufferCount2; i++)
	{
		//s_shaderVecs.constantBuffers[i].ConstantBuffer->Release();
		s_shaderVecsPixelSky.constantBuffers[i].ConstantBuffer = s_shaderVecsSky.constantBuffers[i].ConstantBuffer;
		s_shaderVecsPixelSky.constantBuffers[i].ConstantBuffer->Release();
		delete[] s_shaderVecsPixelSky.constantBuffers[i].LocalDataBuffer;
	}

	if (s_shaderVecsPixelSky.constantBuffers)
	{
		delete[] s_shaderVecsPixelSky.constantBuffers;
	}

	for (unsigned int i = 0; i < s_shaderVecsPixelSky.shaderResourceViews.size(); i++)
		delete s_shaderVecsPixelSky.shaderResourceViews[i];

	for (unsigned int i = 0; i < s_shaderVecsPixelSky.samplerStates.size(); i++)
		delete s_shaderVecsPixelSky.samplerStates[i];

	// Clean up tables
	s_shaderVecsPixelSky.varTable.clear();
	s_shaderVecsPixelSky.cbTable.clear();
	s_shaderVecsPixelSky.samplerTable.clear();
	s_shaderVecsPixelSky.textureTable.clear();

	// Handle constant buffers and local data buffers
	for (unsigned int i = 0; i < constanBufferCount2; i++)
	{
		s_shaderVecsSky.constantBuffers[i].ConstantBuffer->Release();
		delete[] s_shaderVecsSky.constantBuffers[i].LocalDataBuffer;
	}

	if (s_shaderVecsSky.constantBuffers)
	{
		delete[] s_shaderVecsSky.constantBuffers;
		constanBufferCount2 = 0;
	}

	for (unsigned int i = 0; i < s_shaderVecsSky.shaderResourceViews.size(); i++)
		delete s_shaderVecsSky.shaderResourceViews[i];

	for (unsigned int i = 0; i < s_shaderVecsSky.samplerStates.size(); i++)
		delete s_shaderVecsSky.samplerStates[i];

	// Clean up tables
	s_shaderVecsSky.varTable.clear();
	s_shaderVecsSky.cbTable.clear();
	s_shaderVecsSky.samplerTable.clear();
	s_shaderVecsSky.textureTable.clear();
}


// --------------------------------------------------------
// Creates the DirectX vertex shader
//
// shaderBlob - The shader's compiled code
//
// Returns true if shader is created correctly, false otherwise
// --------------------------------------------------------
bool SkyShader::CreateShaderSky(entt::registry& registry)
{

	auto s_mainShaderComp = registry.view<RendererMainVars>();

	auto mycomp = registry.view<SimpleShaderVertexVariablesSky, SkyVarsVertexShader>();

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

	auto [c_simpleShaderVariables, c_simpleVertexShaderStruct] = registry.get<SimpleShaderVertexVariablesSky, SkyVarsVertexShader>(entity2);

	// Create the shader from the blob
	HRESULT result = c_rendererMainVars.device->CreateVertexShader(
		c_simpleShaderVariables.shaderBlob->GetBufferPointer(),
		c_simpleShaderVariables.shaderBlob->GetBufferSize(),
		0,
		&c_simpleVertexShaderStruct.shader);

	// Did the creation work?
	if (result != S_OK)
		return false;

	// Do we already have an input layout?
	if (c_simpleVertexShaderStruct.inputLayout)
		return true;

	// Vertex shader was created successfully, so we now use the
	// shader code to re-reflect and create an input layout that 
	// matches what the vertex shader expects.  Code adapted from:
	// https://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/

	// Reflect shader info
	ID3D11ShaderReflection* refl;
	D3DReflect(
		c_simpleShaderVariables.shaderBlob->GetBufferPointer(),
		c_simpleShaderVariables.shaderBlob->GetBufferSize(),
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

			c_simpleVertexShaderStruct.perInstanceCompatible = true;
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
		c_simpleShaderVariables.shaderBlob->GetBufferPointer(),
		c_simpleShaderVariables.shaderBlob->GetBufferSize(),
		&c_simpleVertexShaderStruct.inputLayout);

	registry.replace<RendererMainVars>(entity1, c_rendererMainVars.swapChain, c_rendererMainVars.device,
		c_rendererMainVars.context, c_rendererMainVars.backBufferRTV, c_rendererMainVars.depthStencilView);

	registry.replace<SkyVarsVertexShader>(entity2, c_simpleVertexShaderStruct.perInstanceCompatible, c_simpleVertexShaderStruct.inputLayout,
		c_simpleVertexShaderStruct.shader);

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
bool SkyShader::CreateShaderPixelSky(entt::registry& registry)
{
	entt::entity gpu_main;

	entt::entity ps_entity;
	auto mycomp = registry.view<RendererMainVars>();

	auto mycomp2 = registry.view<SkyVarsPixelShader, SimpleShaderPixelVariablesSky>();

	for (auto entity : mycomp)
	{
		gpu_main = entity;
	}

	for (auto entity : mycomp2)
	{
		ps_entity = entity;
	}

	auto& c_win = mycomp.get<RendererMainVars>(gpu_main);

	auto [c_pixStruct, c_pixVars] = mycomp2.get<SkyVarsPixelShader, SimpleShaderPixelVariablesSky>(ps_entity);

	// Create the shader from the blob
	HRESULT result = c_win.device->CreatePixelShader(
		c_pixVars.shaderBlob->GetBufferPointer(),
		c_pixVars.shaderBlob->GetBufferSize(),
		0,
		&c_pixStruct.shader);

	registry.replace<SkyVarsPixelShader>(ps_entity, c_pixStruct.shader);

	// Check the result
	return (result == S_OK);
}

// Sky
// Part of creating vertex shader
entt::registry& SkyShader::ReadFileToBlob_VertexSky(entt::registry& registry)
{
	auto mainShaderComp = registry.view<SimpleShaderVertexVariablesSky, ShaderStringsSky>();

	for (auto entity : mainShaderComp)
	{
		auto [mycomp, mycomp2] = mainShaderComp.get<SimpleShaderVertexVariablesSky, ShaderStringsSky>(entity);
		// Load the shader to a blob and ensure it worked
		D3DReadFileToBlob(mycomp2.SkyVertexShaderString, &mycomp.shaderBlob); // also returns HRESULT

		registry.replace<SimpleShaderVertexVariablesSky>(entity, mycomp.shaderValid, mycomp.shaderBlob,
			mycomp.ConstantBuffer, mycomp.constantBufferCount);
	}
	return registry;
}

// Sky
// Part of creating pixel shader
entt::registry& SkyShader::ReadFileToBlob_PixelSky(entt::registry& registry)
{
	auto mainShaderComp = registry.view<SimpleShaderPixelVariablesSky>();

	LPCWSTR abc = L"SkyPS.cso"; // test
	for (auto entity : mainShaderComp)
	{
		auto mycomp = mainShaderComp.get<SimpleShaderPixelVariablesSky>(entity);

		// Load the shader to a blob and ensure it worked
		HRESULT hr = D3DReadFileToBlob(abc, &mycomp.shaderBlob);
		if (hr != S_OK)
		{
			//return false;
		}

		registry.replace<SimpleShaderPixelVariablesSky>(entity, mycomp.shaderValid, mycomp.shaderBlob,
			mycomp.ConstantBuffer, mycomp.constantBufferCount);
	}
	return registry;
}


void SkyShader::SetSkyVars(entt::registry& registry)
{
	auto d_component = registry.view<RendererMainVars>();

	entt::entity temp_SkyVar = registry.create();

	auto s_component = registry.view<SkyVars>();

	for (auto entity : s_component)
	{
		temp_SkyVar = entity;
	}

	auto temp_SkyComp = registry.get<SkyVars>(temp_SkyVar);

	for (auto entity : d_component)
	{
		auto& gpu_component = d_component.get<RendererMainVars>(entity);

		// Load the sky box
		DirectX::CreateDDSTextureFromFile(gpu_component.device, L"Textures/Skybox_1.dds", 0, &temp_SkyComp.skySRV);

		// Create the states for the sky
		D3D11_RASTERIZER_DESC rd = {};
		rd.CullMode = D3D11_CULL_FRONT;
		rd.FillMode = D3D11_FILL_SOLID;
		rd.DepthClipEnable = true;
		gpu_component.device->CreateRasterizerState(&rd, &temp_SkyComp.skyRasterState);

		D3D11_DEPTH_STENCIL_DESC dd = {};
		dd.DepthEnable = true;
		dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		gpu_component.device->CreateDepthStencilState(&dd, &temp_SkyComp.skyDepthState);


		// Tell the input assembler stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our data?"
		gpu_component.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		registry.replace<RendererMainVars>(entity, gpu_component.swapChain, gpu_component.device,
			gpu_component.context, gpu_component.backBufferRTV, gpu_component.depthStencilView);
	}

	registry.replace<SkyVars>(temp_SkyVar, temp_SkyComp.skySRV, temp_SkyComp.skyRasterState, temp_SkyComp.skyDepthState);
}


// -----------------------------------------------------------------------------------------
// Loads the specified vertex shader and builds the variable table using shader reflection.  
//
// shaderFile - A "wide string" specifying the compiled shader to load
// -----------------------------------------------------------------------------------------
entt::registry& SkyShader::LoadShaderFileSky(entt::registry& registry)
{

	bool checkLoadShader = CreateShaderSky(ReadFileToBlob_VertexSky(registry));

	auto d_component = registry.view<RendererMainVars>();

	for (auto entity : d_component)
	{
		auto& get_component = d_component.get<RendererMainVars>(entity);

		auto mainShaderComp = registry.view<SimpleShaderVertexVariablesSky>();

		for (auto entity : mainShaderComp)
		{
			auto mycomp = mainShaderComp.get<SimpleShaderVertexVariablesSky>(entity);

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
			s_shaderVecsSky.constantBuffers = new SimpleConstantBuffer[mycomp.constantBufferCount];

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
					SimpleSRV* srv = new SimpleSRV();
					srv->BindIndex = resourceDesc.BindPoint;				// Shader bind point
					srv->Index = (unsigned int)s_shaderVecsSky.shaderResourceViews.size();	// Raw index

					s_shaderVecsSky.textureTable.insert(std::pair<std::string, SimpleSRV*>(resourceDesc.Name, srv));
					s_shaderVecsSky.shaderResourceViews.push_back(srv);
				}
				break;

				case D3D_SIT_SAMPLER: // A sampler resource
				{
					//Create the sampler wrapper
					SimpleSampler* samp = new SimpleSampler();
					samp->BindIndex = resourceDesc.BindPoint;			// Shader bind point
					samp->Index = (unsigned int)s_shaderVecsSky.samplerStates.size();	// Raw index

					s_shaderVecsSky.samplerTable.insert(std::pair<std::string, SimpleSampler*>(resourceDesc.Name, samp));
					s_shaderVecsSky.samplerStates.push_back(samp);
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
				s_shaderVecsSky.constantBuffers[b].BindIndex = bindDesc.BindPoint;
				s_shaderVecsSky.constantBuffers[b].Name = bufferDesc.Name;
				s_shaderVecsSky.cbTable.insert(std::pair<std::string, SimpleConstantBuffer*>(bufferDesc.Name, &s_shaderVecsSky.constantBuffers[b]));

				// Create this constant buffer
				D3D11_BUFFER_DESC newBuffDesc;
				newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
				newBuffDesc.ByteWidth = bufferDesc.Size;
				newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				newBuffDesc.CPUAccessFlags = 0;
				newBuffDesc.MiscFlags = 0;
				newBuffDesc.StructureByteStride = 0;
				get_component.device->CreateBuffer(&newBuffDesc, 0, &s_shaderVecsSky.constantBuffers[b].ConstantBuffer);

				// Set up the data buffer for this constant buffer
				s_shaderVecsSky.constantBuffers[b].Size = bufferDesc.Size;
				s_shaderVecsSky.constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
				ZeroMemory(s_shaderVecsSky.constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

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
					SimpleShaderVariable varStruct;
					varStruct.ConstantBufferIndex = b;
					varStruct.ByteOffset = varDesc.StartOffset;
					varStruct.Size = varDesc.Size;

					// Get a string version
					std::string varName(varDesc.Name);

					// Add this variable to the table and the constant buffer
					s_shaderVecsSky.varTable.insert(std::pair<std::string, SimpleShaderVariable>(varName, varStruct));
					s_shaderVecsSky.constantBuffers[b].Variables.push_back(varStruct);
				}
			}

			registry.replace<SimpleShaderVertexVariablesSky>(entity, mycomp.shaderValid, mycomp.shaderBlob, mycomp.ConstantBuffer, mycomp.constantBufferCount);

			// All set
			refl->Release();
		}
	}
	return registry;
}


// -----------------------------------------------------------------------------------
// Loads the specified pixel shader and builds the variable table using shader reflection.  
//
// shaderFile - A "wide string" specifying the compiled shader to load
// -----------------------------------------------------------------------------------
entt::registry& SkyShader::LoadPixelShaderFileSky(entt::registry& registry)
{

	bool checkLoadShader = CreateShaderPixelSky(ReadFileToBlob_PixelSky(registry));

	auto d_component = registry.view<RendererMainVars>();

	for (auto entity : d_component)
	{
		RendererMainVars& get_component = d_component.get<RendererMainVars>(entity);

		auto mainShaderComp = registry.view<SimpleShaderPixelVariablesSky>();

		for (auto entity : mainShaderComp)
		{
			auto mycomp = mainShaderComp.get<SimpleShaderPixelVariablesSky>(entity);

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
			s_shaderVecsPixelSky.constantBuffers = new SimpleConstantBuffer[mycomp.constantBufferCount];

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
					SimpleSRV* srv = new SimpleSRV();
					srv->BindIndex = resourceDesc.BindPoint;				// Shader bind point
					srv->Index = (unsigned int)s_shaderVecsPixelSky.shaderResourceViews.size();	// Raw index

					s_shaderVecsPixelSky.textureTable.insert(std::pair<std::string, SimpleSRV*>(resourceDesc.Name, srv));
					s_shaderVecsPixelSky.shaderResourceViews.push_back(srv);
				}
				break;

				case D3D_SIT_SAMPLER: // A sampler resource
				{
					//Create the sampler wrapper
					SimpleSampler* samp = new SimpleSampler();
					samp->BindIndex = resourceDesc.BindPoint;			// Shader bind point
					samp->Index = (unsigned int)s_shaderVecsPixelSky.samplerStates.size();	// Raw index

					s_shaderVecsPixelSky.samplerTable.insert(std::pair<std::string, SimpleSampler*>(resourceDesc.Name, samp));
					s_shaderVecsPixelSky.samplerStates.push_back(samp);
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
				s_shaderVecsPixelSky.constantBuffers[b].BindIndex = bindDesc.BindPoint;
				s_shaderVecsPixelSky.constantBuffers[b].Name = bufferDesc.Name;
				s_shaderVecsPixelSky.cbTable.insert(std::pair<std::string, SimpleConstantBuffer*>(bufferDesc.Name, &s_shaderVecsPixelSky.constantBuffers[b]));

				// Create this constant buffer
				D3D11_BUFFER_DESC newBuffDesc;
				newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
				newBuffDesc.ByteWidth = bufferDesc.Size;
				newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				newBuffDesc.CPUAccessFlags = 0;
				newBuffDesc.MiscFlags = 0;
				newBuffDesc.StructureByteStride = 0;
				get_component.device->CreateBuffer(&newBuffDesc, 0, &s_shaderVecsPixelSky.constantBuffers[b].ConstantBuffer);

				// Set up the data buffer for this constant buffer
				s_shaderVecsPixelSky.constantBuffers[b].Size = bufferDesc.Size;
				s_shaderVecsPixelSky.constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
				ZeroMemory(s_shaderVecsPixelSky.constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

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
					SimpleShaderVariable varStruct;
					varStruct.ConstantBufferIndex = b;
					varStruct.ByteOffset = varDesc.StartOffset;
					varStruct.Size = varDesc.Size;

					// Get a string version
					std::string varName(varDesc.Name);

					// Add this variable to the table and the constant buffer
					s_shaderVecsPixelSky.varTable.insert(std::pair<std::string, SimpleShaderVariable>(varName, varStruct));
					s_shaderVecsPixelSky.constantBuffers[b].Variables.push_back(varStruct);
				}
			}

			registry.replace<SimpleShaderPixelVariablesSky>(entity, mycomp.shaderValid, mycomp.shaderBlob, mycomp.ConstantBuffer, mycomp.constantBufferCount);

			// All set
			refl->Release();
		}
	}
	return registry;
}


// -------------------------------------------------------------------
// Sets a variable by name with arbitrary data of the specified size
// name - The name of the shader variable
// data - The data to set in the buffer
// size - The size of the data (this must match the variable's size)
// -------------------------------------------------------------------
void SkyShader::SetData(std::string name, const void* data, unsigned int size)
{
	// Look for the variable and verify
	SimpleShaderVariable* var = FindVariable(name, size);

	// Set the data in the local data buffer
	memcpy(
		s_shaderVecsSky.constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
		data,
		size);
}


// -------------------------------------------------------------------
// Sets a variable by name with arbitrary data of the specified size
// name - The name of the shader variable
// data - The data to set in the buffer
// size - The size of the data (this must match the variable's size)
// ------------------------------------------------------------------
void SkyShader::SetDataPixel(std::string name, const DirectX::XMFLOAT3 data, unsigned int size)
{
	// Look for the variable and verify
	SimpleShaderVariable* var = FindVariablePixel(name, size);

	// Set the data in the local data buffer
	memcpy(
		s_shaderVecsPixelSky.constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
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
SimpleShaderVariable* SkyShader::FindVariable(std::string name, int size)
{
	// Look for the key
	std::unordered_map<std::string, SimpleShaderVariable>::iterator result =
		s_shaderVecsSky.varTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsSky.varTable.end())
		return 0;

	// Grab the result from the iterator
	SimpleShaderVariable* var = &(result->second);

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
SimpleShaderVariable* SkyShader::FindVariablePixel(std::string name, int size)
{
	// Look for the key
	std::unordered_map<std::string, SimpleShaderVariable>::iterator result =
		s_shaderVecsPixelSky.varTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixelSky.varTable.end())
		return 0;

	// Grab the result from the iterator
	SimpleShaderVariable* var = &(result->second);

	// Is the data size correct ?
	if (size > 0 && var->Size != size)
		return 0;

	// Success
	return var;
}


void SkyShader::FindVariableBasic(std::string name, ID3D11SamplerState* samplerState, ID3D11DeviceContext* deviceContext)
{
	// Look for the key
	std::unordered_map<std::string, SimpleSampler*>::iterator result =
		s_shaderVecsPixelSky.samplerTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixelSky.samplerTable.end())
		return;

	const SimpleSampler* sampInfo = result->second;

	if (sampInfo == 0)
		return;

	// Set the shader resource view
	deviceContext->PSSetSamplers(sampInfo->BindIndex, 1, &samplerState);
}


void SkyShader::FindVariableTexture(std::string name, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* deviceContext)
{
	// Look for the key
	std::unordered_map<std::string, SimpleSRV*>::iterator result =
		s_shaderVecsPixelSky.textureTable.find(name);

	// Did we find the key?
	if (result == s_shaderVecsPixelSky.textureTable.end())
		return;

	const SimpleSRV* srvInfo = result->second;

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
void SkyShader::CopyAllBufferData(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount)
{
	// Ensure the shader is valid
	//if (!shaderValid) return;

	// Loop through the constant buffers and copy all data
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Copy the entire local data buffer
		deviceContext->UpdateSubresource(
			s_shaderVecsSky.constantBuffers[i].ConstantBuffer, 0, 0,
			s_shaderVecsSky.constantBuffers[i].LocalDataBuffer, 0, 0);
	}
}


void SkyShader::CopyAllBufferDataPixel(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount)
{
	// Ensure the shader is valid
	// if (!shaderValid) return;

	// Loop through the constant buffers and copy all data
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Copy the entire local data buffer
		deviceContext->UpdateSubresource(
			s_shaderVecsPixelSky.constantBuffers[i].ConstantBuffer, 0, 0,
			s_shaderVecsPixelSky.constantBuffers[i].LocalDataBuffer, 0, 0);
	}
}


// --------------------------------------------------------
// Sets the vertex shader, input layout and constant buffers
// for future DirectX drawing
// --------------------------------------------------------
void SkyShader::SetShaderAndCBs(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, SkyVarsVertexShader* obj_vsStruct)
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
			s_shaderVecsSky.constantBuffers[i].BindIndex,
			1,
			&s_shaderVecsSky.constantBuffers[i].ConstantBuffer);
	}

	constanBufferCount2 = constantBufferCount;
}


// --------------------------------------------------------
// Sets the vertex shader, input layout and constant buffers
// for future DirectX drawing
// --------------------------------------------------------
void SkyShader::SetShaderAndCBsPixel(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, SkyVarsPixelShader* obj_psStruct)
{
	// Is shader valid?
	//if (!shaderValid) return;

	// Set the shader
	deviceContext->PSSetShader(obj_psStruct->shader, 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		deviceContext->PSSetConstantBuffers(
			s_shaderVecsPixelSky.constantBuffers[i].BindIndex,
			1,
			&s_shaderVecsPixelSky.constantBuffers[i].ConstantBuffer);
	}
}