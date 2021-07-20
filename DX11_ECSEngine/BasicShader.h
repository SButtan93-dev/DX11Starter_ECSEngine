#pragma once
#include "entt/entt.hpp"
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
class BasicShader {
public:
	BasicShader();
	 ~BasicShader();

	entt::registry& InitShaderBegin(entt::registry &registry);

	bool CreateVertexShader(entt::registry& registry);
	bool CreatePixelShader(entt::registry & registry);

	entt::registry& ReadFileToBlobVertex(entt::registry& registry);
	entt::registry & ReadFileToBlobPixel(entt::registry & registry);

	void LoadVertexShaderFile(entt::registry &registry);
	void LoadPixelShaderFile(entt::registry & registry);

	VertexShaderBuffVars s_shaderVecsVertex = {0};
	PixelShaderBuffVars s_shaderVecsPixel = {0};

	void CreateMatrices(entt::registry& registry);

	void SetDataVertex(std::string name, const void* data, unsigned int size);
	void SetDataPixel(std::string name, const DirectX::XMFLOAT3 data, unsigned int size);

	void CopyAllBufferDataVertex(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount);
	void CopyAllBufferDataPixel(ID3D11DeviceContext * deviceContext, bool shaderValid, unsigned int constantBufferCount);

	void SetShaderAndCBsVertex(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, InputLayoutVertexShader* obj_vsStruct);
	void SetShaderAndCBsPixel(bool shaderValid, ID3D11DeviceContext * deviceContext, unsigned int constantBufferCount, PixelShader * obj_psStruct);

	void FindVariableBasic(std::string name, ID3D11SamplerState* samplerState, ID3D11DeviceContext* deviceContext);
	void FindVariableTexture(std::string name, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* deviceContext);

	ShaderVariableInfo* FindVariableVertex(std::string name, int size);
	ShaderVariableInfo* FindVariablePixel(std::string name, int size);
};
