
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix BoneData[52];
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	uint4 BoneIDs		: BLENDINDICES;
	float4 Weights		: BLENDWEIGHT;
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

	matrix BoneTransform = mul(input.Weights.x, BoneData[input.BoneIDs.x]);
	BoneTransform = BoneTransform + mul(input.Weights.y, BoneData[input.BoneIDs.y]);
	BoneTransform = BoneTransform + mul(input.Weights.z, BoneData[input.BoneIDs.z]);
	BoneTransform = BoneTransform + mul(input.Weights.w, BoneData[input.BoneIDs.w]);

	matrix worldViewProj = mul(mul(world, view), projection);
	float4 PosL = mul(float4(input.position, 1.0f), BoneTransform);
	output.position = mul(PosL, worldViewProj);

	// Calculate the world position for this vertex
	output.worldPos = mul(PosL, world).xyz;

	float4 NormL = mul(float4(input.normal, 0.0f), BoneTransform);
	// Transform the normal using the world matrix
	output.normal = mul(NormL, (float3x3)world);

	// Remember to normalize the normal since it's probably also scaled
	output.normal = normalize(output.normal);

	// Pass through the uv
	output.uv = input.uv;

	return output;
}