struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
	float4 res : REST_POSITION;
	float4 axisAngle : AXIS_ANGLE;
};