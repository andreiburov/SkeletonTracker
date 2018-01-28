#define SMPL_SKELETON_POSITION_COUNT 24

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 nor : NORMAL;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 posView : VIEW_SPACE_POSITION;
	float3 norView : VIEW_SPACE_NORMAL;
};