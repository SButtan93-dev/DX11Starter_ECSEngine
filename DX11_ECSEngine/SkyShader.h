#pragma once
#include "entt/entt.hpp"
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "DDSTextureLoader.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "d3d11.lib")

#include "Components.h"




class SkyShader
{

public:

	SkyShader();
	~SkyShader();

	bool CreateShaderSky(entt::registry& registry);

	bool CreateShaderPixelSky(entt::registry& registry);

	entt::registry& ReadFileToBlob_VertexSky(entt::registry& registry);

	entt::registry& ReadFileToBlob_PixelSky(entt::registry& registry);

	void SetSkyVars(entt::registry& registry);

	entt::registry& LoadShaderFileSky(entt::registry& registry);

	entt::registry& LoadPixelShaderFileSky(entt::registry& registry);

	void SetData(std::string name, const void* data, unsigned int size);

	void SetDataPixel(std::string name, const DirectX::XMFLOAT3 data, unsigned int size);

	SimpleShaderVariable* FindVariable(std::string name, int size);

	SimpleShaderVariable* FindVariablePixel(std::string name, int size);

	void FindVariableBasic(std::string name, ID3D11SamplerState* samplerState, ID3D11DeviceContext* deviceContext);

	void FindVariableTexture(std::string name, ID3D11ShaderResourceView* srv, ID3D11DeviceContext* deviceContext);

	void CopyAllBufferData(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount);

	void CopyAllBufferDataPixel(ID3D11DeviceContext* deviceContext, bool shaderValid, unsigned int constantBufferCount);

	void SetShaderAndCBs(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, SkyVarsVertexShader* obj_vsStruct);

	void SetShaderAndCBsPixel(bool shaderValid, ID3D11DeviceContext* deviceContext, unsigned int constantBufferCount, SkyVarsPixelShader* obj_psStruct);

	ShaderVectorsofStructsSky s_shaderVecsSky = { 0 };
	ShaderVectorsofStructsPixelSky s_shaderVecsPixelSky = { 0 };


};

