#include "Common.hlsli"

float4 SimplePixelShader(PixelShaderInput input) : SV_TARGET
{
	return float4(1.0, 0.0, 0.0, 1.0);
	//return input.color;
}