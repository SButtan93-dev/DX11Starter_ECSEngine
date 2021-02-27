

cbuffer externalData : register(b0)
{
	float3 DirLightDirection;
	float3 DirLightColor;
	float3 PointLightPosition;
	float3 PointLightColor;

	float3 CameraPosition;
}

// Texture resources
Texture2D DiffuseTexture	: register(t0);
Texture2D SpecularMap		: register(t1);

SamplerState BasicSampler	: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
};


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Fix for poor normals: re-normalizing interpolated normals
	input.normal = normalize(input.normal);

// Calculate the amount of light diffusing (from a DIRECTIONAL LIGHT)
// using the N dot L (or Lambert) BRDF
// NOTE: This formula assumes we're comparing the direction TO the light
// NOTE: Make sure the normal is in WORLD space, not "local" space
// NOTE: Saturate() clamps a number/vector between 0 and 1
float dirLightAmount = saturate(dot(input.normal, -DirLightDirection));

// POINT LIGHT diffuse calculation
float3 dirToPointLight = normalize(PointLightPosition - input.worldPos);
float pointLightAmount = saturate(dot(input.normal, dirToPointLight));

// POINT LIGHT specular calc
// Calc the reflection vector of the INCOMING light
float3 refl = reflect(-dirToPointLight, input.normal);

// Direction to the camera from the current pixel
float3 dirToCamera = normalize(CameraPosition - input.worldPos);

float pointLightSpecular = pow(saturate(dot(refl, dirToCamera)), 64);

// Handle texture sampling here
float4 surfaceColor = DiffuseTexture.Sample(BasicSampler, input.uv);
float  specAdjust = SpecularMap.Sample(BasicSampler, input.uv).r;

// Adjust the light color by the light amount
return float4(
	// Start with some ambient to tint the surface color
	float3(0.1f, 0.2f, 0.3f) * surfaceColor.rgb +

	DirLightColor * dirLightAmount * surfaceColor.rgb +		// Directional
	PointLightColor * pointLightAmount * surfaceColor.rgb +	// Point
	pointLightSpecular * specAdjust,					// Point Specular
	1);
}