#include "SimpleSkeleton3D.hlsli"

#define SMPL_SKELETON_POSITION_COUNT 24

cbuffer lbsConstantBuffer : register(b0)
{
	matrix transform[SMPL_SKELETON_POSITION_COUNT];
};

struct VertexShaderInput
{
	float4 axisAngle : AXIS_ANGLE;
	float3 pos : POSITION;
};

GeometryShaderInput main(VertexShaderInput input, uint id : SV_VertexID)
{
	GeometryShaderInput output = (GeometryShaderInput)0;
	float4 pos = float4(input.pos, 1.f);
	output.res = pos;
	output.pos = mul(pos, transform[id]);
	return output;
}