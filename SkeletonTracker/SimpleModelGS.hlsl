#include "SimpleModel.hlsli"

cbuffer vertexConstantBuffer : register(b0)
{
	matrix worldView;
	matrix projection;
	matrix worldViewIT; // Inverse Transpose
};

float3 calculateNormal(float3 v0, float3 v1, float3 v2)
{
	float3 u = v0 - v1;
	float3 v = v1 - v2;
	float3 n = normalize(cross(u, v));

	return n;
}

[maxvertexcount(3)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream< PixelShaderInput > output,
	uint id : SV_PrimitiveID
)
{
	for (int i = 0; i < 3; i++)
	{
		PixelShaderInput element = (PixelShaderInput)0;
		float4 posView = mul(input[i], worldView);
		element.posView = posView.xyz;
		float3 normal = calculateNormal(input[0].xyz, input[1].xyz, input[2].xyz);
		element.norView = mul(normal, (float3x3)worldViewIT);
		element.pos = mul(posView, projection);
		output.Append(element);
	}
}