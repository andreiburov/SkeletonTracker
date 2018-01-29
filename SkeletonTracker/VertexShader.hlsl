#include "Common.hlsli"

#define SMPL_SKELETON_POSITION_COUNT 24
#define SMPL_POSDIRS_COUNT 207

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

cbuffer posdirsConstantBuffer : register(b2)
{
	float3 posdirs[SMPL_POSDIRS_COUNT];
}

cbuffer poscoefConstantBuffer : register(b3)
{
	float poscoef[SMPL_POSDIRS_COUNT];
}

struct VertexShaderInput
{
	// for skinning
	float4 wgt : WEIGHTS;
	int4 jdx: JOINT_INDICES;

	float3 pos : POSITION;
	float3 nor : NORMAL;
};

struct LBSOutput
{
	float3 pos : POSITION;
	float3 nor : NORMAL;
};

float3 SMPLPoseCorrection(uint id : SV_VertexID)
{
	float3 pos = posdirs[id];
	return pos;
}

LBSOutput LinearBlendSkinning(VertexShaderInput input)
{
	LBSOutput output = (LBSOutput)0;

	matrix S = input.wgt.x * transform[input.jdx.x]
	+ input.wgt.y * transform[input.jdx.y]
	+ input.wgt.z * transform[input.jdx.z]
	+ input.wgt.w * transform[input.jdx.w];

	output.pos = mul(float4(input.pos, 1.f), S).xyz;
	output.nor = mul(float4(input.nor, 1.f), S).xyz;

	return output;
}

PixelShaderInput SimpleVertexShader(VertexShaderInput input)
{
	PixelShaderInput output = (PixelShaderInput)0;
	LBSOutput lbs = LinearBlendSkinning(input);
	
	float4 posView = mul(float4(lbs.pos, 1.f), worldView);
	output.posView = posView.xyz;
	output.norView = mul(lbs.nor, (float3x3)worldViewIT);
	output.pos = mul(posView, projection);

	return output;
}