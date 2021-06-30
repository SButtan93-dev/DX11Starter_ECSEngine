
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

	//float abc2 = Weights.x;
	//float abc3 = BoneData._11;

	// Calculate output position

	//output.normal.x = BoneData[0]._11;
	//output.normal.y = BoneData[0]._21;
	//output.normal.z = BoneData[0]._31;

	matrix abc = mul(input.Weights.x, BoneData[input.BoneIDs.x]);
	abc = abc + mul(input.Weights.y, BoneData[input.BoneIDs.y]);
	abc = abc + mul(input.Weights.z, BoneData[input.BoneIDs.z]);
	abc = abc + mul(input.Weights.w, BoneData[input.BoneIDs.w]);


	//matrix worldViewProj = mul(mul(world, view), projection);
	//output.position = mul(float4(input.position, 1.0f), worldViewProj);

	matrix worldViewProj = mul(mul(world, view), projection);
	float4 temp = mul(float4(input.position, 1.0f), abc);
	output.position = mul(temp, worldViewProj);

	// Calculate the world position for this vertex
	output.worldPos = mul(temp, world).xyz;

	float4 temp2 = mul(float4(input.normal, 1.0f), abc);
	// Transform the normal using the world matrix
	output.normal = mul(temp2, (float3x3)world);

	// Remember to normalize the normal since it's probably also scaled
	output.normal = normalize(output.normal);

	// Pass through the uv
	output.uv = input.uv;

	return output;
}