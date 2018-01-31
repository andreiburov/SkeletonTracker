struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 posView : VIEW_SPACE_POSITION;
	float3 norView : VIEW_SPACE_NORMAL;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
};