#include "Common.hlsli"

#define SMPL_SKELETON_POSITION_COUNT 24

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

struct VertexShaderInput
{
	// for skinning
	float4 wgt : WEIGHTS;
	int4 jdx: JOINT_INDICES;

	float3 pos : POSITION;
	float3 nor : NORMAL;
};

float3 LinearBlendSkinning(VertexShaderInput input)
{
	matrix S = input.wgt.x * transform[input.jdx.x]
	+ input.wgt.y * transform[input.jdx.y]
	+ input.wgt.z * transform[input.jdx.z]
	+ input.wgt.w * transform[input.jdx.w];

	float4 pos =  mul(float4(input.pos, 1.f), S);
	return pos.xyz;
}

PixelShaderInput SimpleVertexShader(VertexShaderInput input)
{
	PixelShaderInput output = (PixelShaderInput)0;

	float3 pos = LinearBlendSkinning(input);
	//float3 pos = input.pos;
	
	float4 posView = mul(float4(pos, 1.f), worldView);
	output.posView = posView.xyz;
	output.norView = mul(input.nor, (float3x3)worldViewIT);
	output.pos = mul(posView, projection);

	return output;
}