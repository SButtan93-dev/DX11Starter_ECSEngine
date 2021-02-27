#pragma once
#include "entt/src/entt/entt.hpp"
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "d3d11.lib")

#include "Components.h"



// --------------------------------------------------------
// Base abstract class for simplifying shader handling
// --------------------------------------------------------
class ISimpleShader
{
public:
		ISimpleShader();
	 ~ISimpleShader();
	entt::registry& InitShaderBegin(entt::registry &registry);

	bool CreateShader(entt::registry& registry);
	bool CreateShaderPixel(entt::registry & registry);

	entt::registry& ReadFileToBlob(entt::registry& registry);
	entt::registry & ReadFileToBlobPixel(entt::registry & registry);

	entt::registry& LoadShaderFile(entt::registry &registry);
	entt::registry & LoadPixelShaderFile(entt::registry & registry);

	ShaderVectorsofStructs s_shaderVecs = {0};
	ShaderVectorsofStructsPixel s_shaderVecsPixel = {0};

	entt::registry& CreateMatrices(entt::registry& registry);

	// Sets arbitrary shader data
	void SetData(std::string name, const void* data, unsigned int size);
	void SetDataPixel(std::string name, const DirectX::XMFLOAT3 data, unsigned int size);

	void CopyAllBufferData(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount);

	void CopyAllBufferDataPixel(ID3D11DeviceContext * deviceContext, bool shaderValid, unsigned int constantBufferCount);

	void SetShaderAndCBs(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, SimpleVertexShaderStruct* obj_vsStruct);
	void SetShaderAndCBsPixel(bool shaderValid, ID3D11DeviceContext * deviceContext, unsigned int constantBufferCount, SimplePixelShaderStruct * obj_psStruct);

	void FindVariableBasic(std::string name, ID3D11SamplerState* samplerState, ID3D11DeviceContext* deviceContext);
	void FindVariableTexture(std::string name, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* deviceContext);

	// Helpers for finding data by name
	SimpleShaderVariable* FindVariable(std::string name, int size);
	SimpleShaderVariable* FindVariablePixel(std::string name, int size);
};
