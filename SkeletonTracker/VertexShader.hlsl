#include "Common.hlsli"

cbuffer simpleConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

PixelShaderInput SimpleVertexShader(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projection space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	vertexShaderOutput.pos = pos;

	// Pass the vertex color through to the pixel shader.
	//vertexShaderOutput.color = float4(input.color, 1.0f);

	return vertexShaderOutput;
}