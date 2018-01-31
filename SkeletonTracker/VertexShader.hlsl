#include "Common.hlsli"

#define SMPL_SKELETON_POSITION_COUNT 24
#define SMPL_POSEDIRS_COUNT 207

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

cbuffer thetaConstantBuffer : register(b2)
{
	float theta[SMPL_POSEDIRS_COUNT];
};

Buffer<float3> posedirs : register(t0);

struct VertexShaderInput
{
	// for skinning
	float4 wgt : WEIGHTS;
	uint4 jdx: JOINT_INDICES;

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
	float3 output = (float3)0;
	for (uint i = 0; i < SMPL_POSEDIRS_COUNT; i++)
	{
		output += theta[i] * posedirs[id*SMPL_POSEDIRS_COUNT + i];
	}

	return output;
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

PixelShaderInput SimpleVertexShader(VertexShaderInput input, uint id : SV_VertexID)
{
	PixelShaderInput output = (PixelShaderInput)0;
	input.pos += SMPLPoseCorrection(id);
	LBSOutput lbs = LinearBlendSkinning(input);
	
	float4 posView = mul(float4(lbs.pos, 1.f), worldView);
	output.posView = posView.xyz;
	output.norView = mul(lbs.nor, (float3x3)worldViewIT);
	output.pos = mul(posView, projection);

	return output;
}