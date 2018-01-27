#include "Common.hlsli"

float4 SimplePixelShader(PixelShaderInput input) : SV_TARGET
{
	return input.color;
}