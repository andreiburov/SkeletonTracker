struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};