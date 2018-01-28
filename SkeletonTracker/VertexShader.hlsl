#include "Common.hlsli"

cbuffer vertexConstantBuffer : register(b0)
{
	matrix worldView;
	matrix projection;
	matrix worldViewIT; // Inverse Transpose
};

cbuffer hierarchyConstantBuffer : register(b1)
{
	matrix transform[SMPL_SKELETON_POSITION_COUNT];
};

PixelShaderInput SimpleVertexShader(VertexShaderInput input)
{
	PixelShaderInput output = (PixelShaderInput)0;
	
	float4 posView = mul(float4(input.pos, 1.f), worldView);
	output.posView = posView.xyz;
	output.norView = mul(input.nor, (float3x3)worldViewIT);
	output.pos = mul(posView, projection);

	return output;
}