#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")

#include <unordered_map>
#include <vector>
#include <string>


// -------------------------------------------------
// Contains info about a single Sampler in a shader
// -------------------------------------------------
struct SimpleSampler
{
	unsigned int Index;		// The raw index of the Sampler
	unsigned int BindIndex; // The register of the Sampler
};


// ---------------------------------------
// Window screen handle and debug options
// ---------------------------------------
struct RenderWindow
{
	HINSTANCE	hInstance;		// The handle to the application
	std::string titleBarText;	// Custom text in window's title bar
	bool		titleBarStats;	// Show extra stats in title bar?
	HWND hWnd;
};


// --------------------------------------------------------
// Contains info about a single SRV in a shader
// --------------------------------------------------------
struct SimpleSRV
{
	unsigned int Index;		// The raw index of the SRV
	unsigned int BindIndex; // The register of the SRV
};


// ---------------------------------------------------
// Local component, entity could be released once set, 
// for now it exists with the entity in the registry
// ---------------------------------------------------
struct RenderWindowDimensions
{
	// Size of the window's client area
	unsigned int width;
	unsigned int height;
};


// ------------------------------------
// Main GPU variables handled by D3D11
// ------------------------------------
struct RendererMainVars
{
	//D3D_FEATURE_LEVEL		dxFeatureLevel;
	IDXGISwapChain* swapChain = 0;
	ID3D11Device* device = 0;
	ID3D11DeviceContext* context = 0;

	ID3D11RenderTargetView* backBufferRTV = 0;
	ID3D11DepthStencilView* depthStencilView = 0;

	D3D_FEATURE_LEVEL		dxFeatureLevel;
};


// Vertex variables for creating shader
struct SimpleShaderVariables
{
	bool shaderValid;
	ID3DBlob* shaderBlob;
	ID3D11Buffer* ConstantBuffer;
	unsigned int constantBufferCount;
};


// Pixel variables for creating shader
struct SimpleShaderPixelVariables
{
	bool shaderValid;
	ID3DBlob* shaderBlob = 0;
	ID3D11Buffer* ConstantBuffer = 0;
	unsigned int constantBufferCount;
};


// Sky
// Vertex variables for creating shader
struct SimpleShaderVertexVariablesSky
{
	bool shaderValid;
	ID3DBlob* shaderBlob;
	ID3D11Buffer* ConstantBuffer;
	unsigned int constantBufferCount;
};


// Sky
// Pixel variables for creating shader
struct SimpleShaderPixelVariablesSky
{
	bool shaderValid;
	ID3DBlob* shaderBlob = 0;
	ID3D11Buffer* ConstantBuffer = 0;
	unsigned int constantBufferCount;
};


// -------------------------------------------------------------------------------
// - Attempt to create inputLayout for the vertex shader
// - Can be loaded for more shaders for every material, for now it is 
//   attaching with 1 vs and ps i.e. the two .hlsl files
// -------------------------------------------------------------------------------
struct SimpleVertexShaderStruct
{
	bool perInstanceCompatible;
	ID3D11InputLayout* inputLayout = 0;
	ID3D11VertexShader* shader = 0;
};


// --------------------------------------------------------------------
// Pixel shader
// - Can be loaded for more shaders for every material, for now it is 
//   attaching with 1 vs and ps i.e. the two .hlsl files
// --------------------------------------------------------------------
struct SimplePixelShaderStruct
{
	ID3D11PixelShader* shader = 0;
};


// Sky Vertex Shader
struct SkyVarsVertexShader
{
	bool perInstanceCompatible;
	ID3D11InputLayout* inputLayout = 0;
	ID3D11VertexShader* shader = 0;
};


// Sky Pixel Shader
struct SkyVarsPixelShader
{
	ID3D11PixelShader* shader = 0;
};


// ------------------------------------------------------------------------
// - String names passed for creating the two shaders
// - Could create the instances of same component 
//   and attach to the same entity to load more shader files in 1 for loop
// ------------------------------------------------------------------------
struct ShaderStrings
{
	LPCWSTR vertexShaderString;
	LPCWSTR pixelShaderString;
};

// ------------------------------------------------------------------------
// - String names passed for creating the two shaders
// - Could create the instances of same component 
//   and attach to the same entity to load more shader files in 1 for loop
// ------------------------------------------------------------------------
struct ShaderStringsSky
{
	LPCWSTR SkyVertexShaderString;
	LPCWSTR SkyPixelShaderString;
};


// --------------------------------------------------------
// Used by simple shaders to store information about
// specific variables in constant buffers
// --------------------------------------------------------
struct SimpleShaderVariable
{
	unsigned int ByteOffset;
	unsigned int Size;
	unsigned int ConstantBufferIndex;
};


// ------------------------------------------
// Contains information about a specific
// constant buffer in a shader, as well as
// the local data buffer for it
// ------------------------------------------
struct SimpleConstantBuffer
{
	std::string Name;
	unsigned int Size;
	unsigned int BindIndex;
	ID3D11Buffer* ConstantBuffer = 0;
	unsigned char* LocalDataBuffer;
	std::vector<SimpleShaderVariable> Variables;
};


// ---------------------------Part (1/2)---------------------------------------
// - Vertex shader contents extracted from .hlsl
// - Expand for more vertex shaders and search from library with key
// ----------------------------------------------------------------------------
struct ShaderVectorsofStructs
{
	SimpleConstantBuffer* constantBuffers = 0; // For index-based lookup
	std::vector<SimpleSRV*>		shaderResourceViews = {0};
	std::vector<SimpleSampler*>	samplerStates = {0};
	std::unordered_map<std::string, SimpleConstantBuffer*> cbTable;
	std::unordered_map<std::string, SimpleShaderVariable> varTable;
	std::unordered_map<std::string, SimpleSRV*> textureTable;
	std::unordered_map<std::string, SimpleSampler*> samplerTable;
};


// ------------------------------Part (2/2)---------------------------------
// - Pixel shader contents extracted from .hlsl
// - Expand for more vertex shaders and search from library with key
// -------------------------------------------------------------------------
struct ShaderVectorsofStructsPixel
{
	SimpleConstantBuffer* constantBuffers; // For index-based lookup
	std::vector<SimpleSRV*>		shaderResourceViews;
	std::vector<SimpleSampler*>	samplerStates;
	std::unordered_map<std::string, SimpleConstantBuffer*> cbTable;
	std::unordered_map<std::string, SimpleShaderVariable> varTable;
	std::unordered_map<std::string, SimpleSRV*> textureTable;
	std::unordered_map<std::string, SimpleSampler*> samplerTable;
};


//-------------------------Sky Resources---------------------------------------
// ---------------------------Part (1/2)---------------------------------------
// - Vertex shader contents extracted from .hlsl
// - Expand for more vertex shaders and search from library with key
// ----------------------------------------------------------------------------
struct ShaderVectorsofStructsSky
{
	SimpleConstantBuffer* constantBuffers = 0; // For index-based lookup
	std::vector<SimpleSRV*>		shaderResourceViews = { 0 };
	std::vector<SimpleSampler*>	samplerStates = { 0 };
	std::unordered_map<std::string, SimpleConstantBuffer*> cbTable;
	std::unordered_map<std::string, SimpleShaderVariable> varTable;
	std::unordered_map<std::string, SimpleSRV*> textureTable;
	std::unordered_map<std::string, SimpleSampler*> samplerTable;
};


//------------------------------Sky Resources-------------------------------
// ------------------------------Part (2/2)---------------------------------
// - Pixel shader contents extracted from .hlsl
// - Expand for more vertex shaders and search from library with key
// -------------------------------------------------------------------------
struct ShaderVectorsofStructsPixelSky
{
	SimpleConstantBuffer* constantBuffers; // For index-based lookup
	std::vector<SimpleSRV*>		shaderResourceViews;
	std::vector<SimpleSampler*>	samplerStates;
	std::unordered_map<std::string, SimpleConstantBuffer*> cbTable;
	std::unordered_map<std::string, SimpleShaderVariable> varTable;
	std::unordered_map<std::string, SimpleSRV*> textureTable;
	std::unordered_map<std::string, SimpleSampler*> samplerTable;
};


// ----------------------Old struct, do not need anymore-----------------------------------
// - mesh worldMatrix is passed every frame to vertex shader
// - The view and projection matrices are passed by the camera
// - Old version supported this struct but replaced with mesh and camera components.
// - Could still be used without the two components, make sure to add to registry in Init().
// -----------------------------------------------------------------------------------------
struct WorldMatrices
{
	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
};


// ---------------------------------------------
// Store mesh buffers after loading into device.
// ---------------------------------------------
struct MeshRenderVars
{
	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	int numIndices;
};


// Animation stuff
struct aiNode
{
	std::string name;
	DirectX::XMFLOAT4X4 Transformation;
	aiNode* Parent;
	aiNode* child;
};

struct aiNodeAnim
{
	std::string name;
	DirectX::XMFLOAT3 positions;
	aiQuaternion rotations;
	DirectX::XMFLOAT3 scale;

};

struct aiAnimation
{
	double duration;
	double TicksPerSecond
};
// Sky mesh
// ---------------------------------------------
// Store mesh buffers after loading into device.
// ---------------------------------------------
struct MeshRenderVarsSky
{
	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	int numIndices;
};

// Sky stuff!
struct SkyVars
{
	ID3D11ShaderResourceView* skySRV;
	ID3D11RasterizerState* skyRasterState;
	ID3D11DepthStencilState* skyDepthState;
};


// -------------------------
// Calculate for each frame
// -------------------------
struct TimeData
{
	// Timing related data
	double perfCounterSeconds;
	float totalTime;
	float deltaTime;
	__int64 startTime; 
	__int64 currentTime;
	__int64 previousTime;
};


// -----------------
// window titlebar
// -----------------
struct FPSData
{
	// FPS calculation
	int fpsFrameCount;
	float fpsTimeElapsed;
};


// ---------------------------------------------------------
// A custom vertex definition
// - Calculate and store position, uv space and normal 
//	 to the camera for each mesh entity passed to the shader
// - Updated to support lighting and eventually textures
// ---------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The position of the vertex
	DirectX::XMFLOAT2 UV;           // UV Coordinate for texturing 
	DirectX::XMFLOAT3 Normal;       // Normal for lighting
};


// -------------------------------------------------------------
// - Pass 'worldMatrix' to vertex shader
// each frame for each entity
// - Could change transform component for each mesh every frame
// -------------------------------------------------------------
struct MeshEntityData
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
};


// -------------------------------------------------------------
// - Pass 'worldMatrix' to vertex shader
// each frame for each entity
// - Could change transform component for each mesh every frame
// -------------------------------------------------------------
struct MeshEntityDataSky
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
};


// -------------------
// Texture resources
// -------------------
struct TextureData
{
	ID3D11ShaderResourceView* crateSRV; // Need one of these PER TEXTURE!
	ID3D11ShaderResourceView* rustSRV;
	ID3D11ShaderResourceView* specSRV;
	ID3D11SamplerState* sampler;	// Need at LEAST one per program (special fx may require their own)
};


// -----------------------------------------------------------
// - Pass view & proj matrices each frame to vertex shader
// - Use transforms with windows call proc 'ProcessMessage()' 
//   to modify data before sending to GPU
// - Need to implement logic for rotations
// -----------------------------------------------------------
struct CameraComponents
{
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	// Transformations
	DirectX::XMFLOAT3 startPosition;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	float xRotation;
	float yRotation;
};


// NA
class Components
{
public:
	Components();
	~Components();
};
