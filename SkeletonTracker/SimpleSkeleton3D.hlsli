struct PixelShaderInput
{
	float4 pos : SV_POSITION;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
	float4 res : REST_POSITION;
};