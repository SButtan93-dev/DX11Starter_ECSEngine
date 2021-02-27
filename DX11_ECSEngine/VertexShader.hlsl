
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Calculate the world position for this vertex
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;

	// Transform the normal using the world matrix
	output.normal = mul(input.normal, (float3x3)world);

	// Remember to normalize the normal since it's probably also scaled
	output.normal = normalize(output.normal);

	// Pass through the uv
	output.uv = input.uv;

	return output;
}